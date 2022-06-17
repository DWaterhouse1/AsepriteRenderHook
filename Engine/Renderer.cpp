#include "Renderer.h"

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace wrengine
{
	Renderer::Renderer(
		const uint32_t width,
		const uint32_t height,
		const std::string& windowName) :
		m_width{ width },
		m_height{ height },
		m_windowName{ windowName }
	{
		loadModels();
		createPipelineLayout();
		recreateSwapchain();
		createCommandBuffers();
	}

	Renderer::~Renderer()
	{
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}

	// Interface start -----------------

	bool Renderer::windowShouldClose()
	{
		return m_window.shouldClose();
	}

	void Renderer::drawFrame()
	{
		uint32_t imageIndex;
		VkResult result = m_swapchain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapchain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("unable to aquire next image!");
		}

		recordCommandBuffer(imageIndex);
		result = m_swapchain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
		if (result ==
			VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR ||
			m_window.wasResized())
		{
			m_window.resetWindowResizeFlag();
			recreateSwapchain();
			return;
		}
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("unable to submit command buffers!");
		}
	}

	void Renderer::waitIdle()
	{
		vkDeviceWaitIdle(m_device.device());
	}
	//  Interface end  -----------------

	void Renderer::loadModels()
	{
		std::vector<Model::Vertex> vertices
		{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		m_model = std::make_unique<Model>(m_device, vertices);
	}

	void Renderer::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.setLayoutCount = 0;
		createInfo.pSetLayouts = nullptr;
		createInfo.pushConstantRangeCount = 0;
		createInfo.pPushConstantRanges = nullptr;
		
		if (vkCreatePipelineLayout(m_device.device(), &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void Renderer::createPipeline()
	{
		assert(m_swapchain != nullptr && "cannot create pipeline before swapchain!");
		assert(m_pipelineLayout != nullptr && "cannot create pipeline before pipeline layout!");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = m_swapchain->getRenderPass();
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pipeline = std::make_unique<Pipeline>(
			m_device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
		);
	}

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
			m_swapchain = std::make_unique<Swapchain>(m_device, extent, std::move(m_swapchain));
			if (m_swapchain->imageCount() != m_commandBuffers.size())
			{
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
		createPipeline();
	}

	void Renderer::recordCommandBuffer(uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapchain->getRenderPass();
		renderPassInfo.framebuffer = m_swapchain->getFrameBuffer(imageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapchain->getSwapChainExtent();
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapchain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_swapchain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_swapchain->getSwapChainExtent() };
		vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

		m_pipeline->bind(m_commandBuffers[imageIndex]);

		m_model->bind(m_commandBuffers[imageIndex]);
		m_model->draw(m_commandBuffers[imageIndex]);

		vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command bufffer!");
		}
	}

	void Renderer::createCommandBuffers()
	{
		m_commandBuffers.resize(m_swapchain->imageCount());

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
} // namespace wrengine