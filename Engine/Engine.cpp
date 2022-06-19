#include "Engine.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "RenderSystem.h"

// std
#include <stdexcept>
#include <array>
#include <iostream>


namespace wrengine
{
	Engine::Engine(
		const uint32_t width,
		const uint32_t height,
		const std::string& windowName) :
		m_width{ width },
		m_height{ height },
		m_windowName{ windowName }
	{
		loadEntities();
	}

	Engine::~Engine()
	{
		vkDestroyDescriptorPool(m_device.device(), m_imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui::DestroyContext();
	}

	// Interface start ----------------------------------
	bool Engine::windowShouldClose()
	{
		return m_window.shouldClose();
	}

	void Engine::waitIdle()
	{
		vkDeviceWaitIdle(m_device.device());
	}

	void Engine::initImGui()
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

		ImGui_ImplVulkan_Init(&initInfo, m_renderer.getSwapchainRenderPass());

		VkCommandBuffer fontCommandBuffer = m_device.beginSingleTimeCommands();

		ImGui_ImplVulkan_CreateFontsTexture(fontCommandBuffer);

		vkEndCommandBuffer(fontCommandBuffer);
		m_device.endSingleTimeCommands(fontCommandBuffer);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void Engine::mainLoop()
	{
		RenderSystem renderSystem{m_device, m_renderer.getSwapchainRenderPass()};

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

			if (auto commandBuffer = m_renderer.beginFrame())
			{

				// in future, additional render passes can be emplaced here
				
				m_renderer.beginSwapchainRenderPass(commandBuffer);
				renderSystem.renderEntities(commandBuffer, m_entities);
				m_renderer.endSwapchainRenderPass(commandBuffer);
				m_renderer.endFrame();
			}
		}
		waitIdle();
	}
	//  Interface end  ----------------------------------

	void Engine::loadEntities()
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
} // namespace wrengine