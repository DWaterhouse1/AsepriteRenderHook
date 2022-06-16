#include "Renderer.h"

// std
#include <stdexcept>
#include <array>

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
		createPipeline();
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
		VkResult result = m_swapchain.acquireNextImage(&imageIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("unable to aquire next image!");
		}

		result = m_swapchain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
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
			{{0.0f, -0.5f}},
			{{0.5f, 0.5f}},
			{{-0.5f, 0.5f}}
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
		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig, m_swapchain.width(), m_swapchain.height());
		pipelineConfig.renderPass = m_swapchain.getRenderPass();
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pipeline = std::make_unique<Pipeline>(
			m_device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
			);
	}

	void Renderer::createCommandBuffers()
	{
		m_commandBuffers.resize(m_swapchain.imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (int i = 0; i < m_commandBuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_swapchain.getRenderPass();
			renderPassInfo.framebuffer = m_swapchain.getFrameBuffer(i);
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_swapchain.getSwapChainExtent();
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			m_pipeline->bind(m_commandBuffers[i]);

			m_model->bind(m_commandBuffers[i]);
			m_model->draw(m_commandBuffers[i]);

			vkCmdEndRenderPass(m_commandBuffers[i]);
			if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command bufffer!");
			}
		}
	}
} // namespace wrengine