#include "Renderer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>

struct SimplePushConstantData
{
	glm::mat2 transform{1.0f};
	glm::vec2 offset;
	alignas(16) glm::vec3 color;
};

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
		loadEntities();
		createPipelineLayout();
		recreateSwapchain();
		createCommandBuffers();
	}

	Renderer::~Renderer()
	{
		vkDestroyDescriptorPool(m_device.device(), m_imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui::DestroyContext();
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}

	// Interface start ----------------------------------
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
		if (
			result == VK_ERROR_OUT_OF_DATE_KHR ||
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

	void Renderer::initImGui()
	{
		// 1. create IMGUI descriptor pool, not 1000 is oversized
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000;
		poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
		poolInfo.pPoolSizes = poolSizes;

		if (vkCreateDescriptorPool(m_device.device(), &poolInfo, nullptr, &m_imguiPool) != VK_SUCCESS)
		{
			throw std::runtime_error("unable to create imgui descriptor pool!");
		}

		// 2. initialize imgui
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		ImGui::StyleColorsDark();

		m_window.windowInitImGui(true);
		ImGui_ImplVulkan_InitInfo initInfo{};
		m_device.setImguiInfo(initInfo);
		initInfo.DescriptorPool = m_imguiPool;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		
		ImGui_ImplVulkan_Init(&initInfo, m_swapchain->getRenderPass());

		VkCommandBuffer fontCommandBuffer = m_device.beginSingleTimeCommands();

		ImGui_ImplVulkan_CreateFontsTexture(fontCommandBuffer);

		vkEndCommandBuffer(fontCommandBuffer);
		m_device.endSingleTimeCommands(fontCommandBuffer);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void Renderer::mainLoop()
	{
		while (!windowShouldClose())
		{
			glfwPollEvents();

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//ImGui::ShowDemoWindow();

			ImGui::Begin("Demo window");
			ImGui::Button("hello!");
			ImGui::End();

			ImGui::Render();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			drawFrame();
		}
		waitIdle();
	}
	//  Interface end  ----------------------------------

	void Renderer::loadEntities()
	{
		std::vector<Model::Vertex> vertices
		{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto model = std::make_shared<Model>(m_device, vertices);
		
		auto triangle = Entity::createEntity();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2D.translation.x = 0.2f;
		triangle.transform2D.scale = { 2.0f, 0.5f };
		triangle.transform2D.rotation = 0.25f * glm::two_pi<float>();
		m_entities.push_back(std::move(triangle));
	}

	void Renderer::createPipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags =
			VK_SHADER_STAGE_VERTEX_BIT |
			VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.setLayoutCount = 0;
		createInfo.pSetLayouts = nullptr;
		createInfo.pushConstantRangeCount = 1;
		createInfo.pPushConstantRanges = &pushConstantRange;
		
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
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
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

		renderEntities(m_commandBuffers[imageIndex]);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_commandBuffers[imageIndex]);

		vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command bufffer!");
		}
	}

	void Renderer::renderEntities(VkCommandBuffer commandBuffer)
	{
		m_pipeline->bind(commandBuffer);
		for (auto& entity : m_entities)
		{
			entity.transform2D.rotation = glm::mod(entity.transform2D.rotation + 0.001f, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.offset = entity.transform2D.translation;
			push.color = entity.color;
			push.transform = entity.transform2D.mat2();//entity.transform2D.mat2();

			vkCmdPushConstants(
				commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			entity.model->bind(commandBuffer);
			entity.model->draw(commandBuffer);
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