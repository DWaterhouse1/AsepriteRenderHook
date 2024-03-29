#include "Swapchain.h"

// std
#include <limits>
#include <iostream>
#include <algorithm>

namespace wrengine
{
Swapchain::Swapchain(Device& deviceRef, VkExtent2D extent) :
	m_device{ deviceRef },
	m_extent{ extent }
{
	init();
}

Swapchain::Swapchain(
	Device& deviceRef,
	VkExtent2D extent,
	std::shared_ptr<Swapchain> previous) :
	m_device{ deviceRef },
	m_extent{ extent },
	m_oldSwapchain{ previous }
{
	init();
	m_oldSwapchain = nullptr;
}

/**
* Initializes the Swapchain object.
*/
void Swapchain::init()
{
	createSwapChain();
	createImageViews();
	createDepthResources();
	createRenderPass();
	createFramebuffers();
	createSyncObjects();
}

Swapchain::~Swapchain()
{
	if (m_swapchain != nullptr)
	{
		vkDestroySwapchainKHR(m_device.device(), m_swapchain, nullptr);
		//m_swapchain = nullptr;
	}

	for (auto imageView : m_imageViews)
	{
		vkDestroyImageView(m_device.device(), imageView, nullptr);
	}
	m_imageViews.clear();

	for (int i = 0; i < m_depthImages.size(); ++i)
	{
		vkDestroyImageView(m_device.device(), m_depthImageViews[i], nullptr);
		vkDestroyImage(m_device.device(), m_depthImages[i], nullptr);
		vkFreeMemory(m_device.device(), m_depthImageMemorys[i], nullptr);
	}

	for (auto framebuffer : m_swapchainFramebuffers)
	{
		vkDestroyFramebuffer(m_device.device(), framebuffer, nullptr);
	}

	vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(
			m_device.device(),
			m_renderFinishedSemaphores[i],
			nullptr);
		vkDestroySemaphore(
			m_device.device(),
			m_imageAvailableSemaphores[i],
			nullptr);
		vkDestroyFence(m_device.device(), m_inFlightFences[i], nullptr);
	}
}

/**
* Chooses a suitable format from a list of available formats. The function
* requires a BGRA8888 UNORM format and SRGB nonlinear color space.
*/
VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}
	
	return availableFormats[0];
}

/**
* Chooses the present mode. Will opt for Mailbox if available, otherwise FIFO.
* 
* @param availablePresentModes Vector of present modes from which to choose.
* @return The chosen present mode.
*/
VkPresentModeKHR Swapchain::chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			std::cout << "present mode: mailbox\n";
			return presentMode;
		}
	}
	std::cout << "present mode: v-sync\n";
	return VK_PRESENT_MODE_FIFO_KHR;
}

/**
* Chooses the suitable swapchain extent, given a list of surface capabilities.
* 
* @param capabilities Surface capabilities against which choice of swap extent
* is made.
* 
* @return The chosen swapchain extent.
*/
VkExtent2D Swapchain::chooseSwapExtent(
	const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D extent = m_extent;
		m_extent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, m_extent.width));
		m_extent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, m_extent.height));

		return extent;
	}
	return VkExtent2D();
}

/**
* Finds the depth format supported by the physical device.
* 
* @return Depth format.
*/
VkFormat Swapchain::findDepthFormat()
{
	return m_device.findSupportedFormat(
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

/**
* Calculates the aspect ratio of the current Swapchain extent
* 
* @return The width:height aspect ratio of the swapchain extent.
*/
float Swapchain::extentAspectRatio()
{
	return
		static_cast<float>(m_swapchainExtent.width) /
		static_cast<float>(m_swapchainExtent.height);
}

/**
* Aquares the next Swapchain image, after waiting for end of flight of the
* current frame.
* 
* @return Result of the next image acquisition.
*/
VkResult Swapchain::acquireNextImage(uint32_t* imageIndex)
{
	vkWaitForFences(
		m_device.device(),
		1,
		&m_inFlightFences[m_currentFrame],
		VK_TRUE,
		std::numeric_limits<uint64_t>::max());

	VkResult result = vkAcquireNextImageKHR(
		m_device.device(),
		m_swapchain,
		std::numeric_limits<uint64_t>::max(),
		m_imageAvailableSemaphores[m_currentFrame],  // must be a not signaled semaphore
		VK_NULL_HANDLE,
		imageIndex);

	return result;
}

/**
* After waiting for image flight completion, resets these fences and submits the
* provided command buffers. Increments the current image index. Will throw a
* runtime error if unable to sumbit the render commands to the graphics queue.
* On success, presents the image.
* 
* @param buffers Pointer to command buffers to be submitted.
* @imageIndex Pointer to current image index.
* 
* @return Result of image presentation.
*/
VkResult Swapchain::submitCommandBuffers(
	const VkCommandBuffer* buffers,
	uint32_t* imageIndex)
{
	if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(
			m_device.device(),
			1,
			&m_imagesInFlight[*imageIndex],
			VK_TRUE, UINT64_MAX);
	}
	m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] =
	{
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = buffers;

	VkSemaphore signalSemaphores[] = 
	{
		m_renderFinishedSemaphores[m_currentFrame]
	};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]);
	if (vkQueueSubmit(
		m_device.graphicsQueue(),
		1,
		&submitInfo,
		m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = imageIndex;

	VkResult result = vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return result;
}

/**
* Compares if dfepth and image formats of the supplied Swapchain object are
* identical.
* 
* @param swapchain The Swapchain object whose formats will be compared.
* 
* @return Result of the comparison.
*/
bool Swapchain::compareSwapFormats(const Swapchain& swapchain) const
{
	return
		swapchain.getSwapChainDepthFormat() == m_swapchainDepthFormat &&
		swapchain.getSwapChainImageFormat() == m_swapchainImageFormat;
}

/**
* Creates the VkSwapchain object. Will throw a runtime error if unsuccessful.
*/
void Swapchain::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = m_device.getSwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat =
		chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
		chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_device.surface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = m_device.findPhysicalQueueFamilies();
	uint32_t queueFamilyIndices[] =
	{
		indices.graphicsFamily,
		indices.presentFamily
	};

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain =
		m_oldSwapchain == nullptr ?
		VK_NULL_HANDLE :
		m_oldSwapchain->m_swapchain;

	if (vkCreateSwapchainKHR(
		m_device.device(),
		&createInfo,
		nullptr,
		&m_swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swapchain!");
	}

	vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &imageCount, m_swapchainImages.data());
	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;
}

/**
* Creates views for the swapchain images. Will throw a runtime error if
* unsuccessful.
*/
void Swapchain::createImageViews()
{
	m_imageViews.resize(m_swapchainImages.size());
	for (size_t i = 0; i < m_swapchainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(
			m_device.device(),
			&createInfo,
			nullptr,
			&m_imageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("unable to create image views!");
		}
	}
}

/**
* Creates depth format, image and image views for the depth buffer. Will throw
* a runtime error if image view creation fails.
*/
void Swapchain::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();
	m_swapchainDepthFormat = depthFormat;
	VkExtent2D swapChainExtent = getSwapChainExtent();

	m_depthImages.resize(imageCount());
	m_depthImageMemorys.resize(imageCount());
	m_depthImageViews.resize(imageCount());

	for (int i = 0; i < m_depthImages.size(); i++) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = swapChainExtent.width;
		imageInfo.extent.height = swapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0;

		m_device.createImageWithInfo(
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_depthImages[i],
			m_depthImageMemorys[i]);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_depthImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(
			m_device.device(),
			&viewInfo,
			nullptr,
			&m_depthImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

/**
* Creates the VkRenderPass object. Will throw a runtime error if unsuccessful.
*/
void Swapchain::createRenderPass()
{
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstSubpass = 0;
	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments =
	{
		colorAttachment,
		depthAttachment
	};
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(
		m_device.device(),
		&renderPassInfo,
		nullptr,
		&m_renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

/**
* Creates frame buffers for each image, with the corresponding image view and
* depth attachments. Will throw a runtime error on failure to create any of the
* frame buffers.
*/
void Swapchain::createFramebuffers()
{
	m_swapchainFramebuffers.resize(imageCount());

	for (size_t i = 0; i < imageCount(); ++i)
	{
		std::array<VkImageView, 2> attachments =
		{
			m_imageViews[i],
			m_depthImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapchainExtent.width;
		framebufferInfo.height = m_swapchainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(
			m_device.device(),
			&framebufferInfo,
			nullptr,
			&m_swapchainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

/**
* Creates the fences and semaaphores necessary for synchronizing the swapchain.
* Will throw a runtime error if creation of any of these objects fails.
*/
void Swapchain::createSyncObjects()
{
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (
			vkCreateSemaphore(
				m_device.device(),
				&semaphoreInfo,
				nullptr,
				&m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(
				m_device.device(),
				&semaphoreInfo,
				nullptr,
				&m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(
				m_device.device(),
				&fenceInfo,
				nullptr,
				&m_inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}
}