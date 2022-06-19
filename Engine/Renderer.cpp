#include "Renderer.h"

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace wrengine
{
	Renderer::Renderer(Window& window, Device& device) : m_window{ window }, m_device{ device }
	{
		recreateSwapchain();
		createCommandBuffers();
	}

	Renderer::~Renderer()
	{
		freeCommandBuffers();
	}

	// Interface start ----------------------------------
	void Renderer::waitIdle()
	{
		vkDeviceWaitIdle(m_device.device());
	}

	VkCommandBuffer Renderer::getCurrentCommandBuffer() const
	{
		assert(m_isFrameStarted && "Cannot get command buffer when frame is not in progress");
		return m_commandBuffers[m_currentFrameIndex];
	}

	int Renderer::getFrameIndex() const
	{
		assert(m_isFrameStarted && "Cannot get frame index when frame is not in progress");
		return m_currentFrameIndex;
	}
	//  Interface end  ----------------------------------

	void Renderer::recreateSwapchain()
	{
		VkExtent2D extent = m_window.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = m_window.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_device.device());
		if (!m_swapchain)
		{
			m_swapchain = std::make_unique<Swapchain>(m_device, extent);
		}
		else
		{
			std::shared_ptr<Swapchain> oldSwapchain = std::move(m_swapchain);
			m_swapchain = std::make_unique<Swapchain>(m_device, extent, oldSwapchain);

			if (!oldSwapchain->compareSwapFormats(*m_swapchain.get()))
			{
				// TODO notify engine to recreate swapchain
				throw std::runtime_error("swap chain format has changed!");
			}
		}
	}

	void Renderer::createCommandBuffers()
	{
		m_commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void Renderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_device.device(),
			m_device.getCommandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data()
		);
		m_commandBuffers.clear();
	}

	VkCommandBuffer Renderer::beginFrame()
	{
		assert(!m_isFrameStarted && "can't call begin frame with frame already in progress");
		VkResult result = m_swapchain->acquireNextImage(&m_currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapchain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("unable to aquire next image!");
		}

		m_isFrameStarted = true;

		VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	void Renderer::endFrame()
	{
		assert(m_isFrameStarted && "can't end frame while frame is not in progress");
		VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command bufffer!");
		}

		VkResult result = m_swapchain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
		if (
			result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR ||
			m_window.wasResized())
		{
			m_window.resetWindowResizeFlag();
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("unable to submit command buffers!");
		}

		m_isFrameStarted = false;
		m_currentFrameIndex = (m_currentFrameIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::beginSwapchainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "cannot begin render pass when frame not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "can't begin render pass on command buffer from a different frame");

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapchain->getRenderPass();
		renderPassInfo.framebuffer = m_swapchain->getFrameBuffer(m_currentImageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapchain->getSwapChainExtent();
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapchain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_swapchain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_swapchain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::endSwapchainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "cannot end render pass when frame not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "can't end render pass on command buffer from a different frame");

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		vkCmdEndRenderPass(commandBuffer);
	}

} // namespace wrengine