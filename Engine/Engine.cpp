#include "Engine.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "RenderSystem.h"

#include <imgui.h>

// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <tuple>
#include <chrono>

namespace wrengine
{
struct GlobalUbo
{
	//glm::mat2 transform{ 1.0f };
	//glm::vec4 offset{ 0.0f };
	glm::vec4 color{ 1.0f, 1.0f, 0.0f, 1.0f};
};

	Engine::Engine(
		const uint32_t width,
		const uint32_t height,
		const std::string& windowName) :
		m_width{ width },
		m_height{ height },
		m_windowName{ windowName }
	{
		m_globalDescriptorPool = 
			DescriptorPool::Builder(m_device)
			.setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				Swapchain::MAX_FRAMES_IN_FLIGHT)
			.build();

		loadEntities();
	}

	Engine::~Engine()
	{
	}

	// Interface start ----------------------------------
	void Engine::run()
	{
		loadTextures();

		std::vector<std::unique_ptr<Buffer>> uboBuffers(
			Swapchain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); ++i)
		{
			uboBuffers[i] = std::make_unique<Buffer>(
				m_device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(
				0,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT)
			.build();
		
		std::vector<VkDescriptorSet> globalDescriptorSets(
			Swapchain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < globalDescriptorSets.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*globalSetLayout, *m_globalDescriptorPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		RenderSystem renderSystem{ 
			m_device,
			m_renderer.getSwapchainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};

		UserInterface userInterface{
			m_window,
			m_device,
			m_renderer.getSwapchainRenderPass(),
			static_cast<uint32_t>(m_renderer.getImageCount())
		};

		std::chrono::steady_clock::time_point currentTime =
			std::chrono::high_resolution_clock::now();

		while (!m_window.shouldClose())
		{
			// note glfwPollEvents() may block, e.g. on window resize
			glfwPollEvents();

			// -------------- frame timing --------------
			std::chrono::steady_clock::time_point newTime =
				std::chrono::high_resolution_clock::now();
			
			float frameTime = 
				std::chrono::duration<float, std::chrono::seconds::period>(
					newTime - currentTime)
				.count();
			currentTime = newTime;

			// -------------  begin frame --------------
			if (VkCommandBuffer commandBuffer = m_renderer.beginFrame())
			{
				int frameIndex = m_renderer.getFrameIndex();
				FrameInfo frameInfo
				{
					frameIndex,
					frameTime,
					commandBuffer,
					globalDescriptorSets[frameIndex]
				};

				// FRAME PHASE 1
				// prepare and update objects in memory
				GlobalUbo ubo{};
				ubo.color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// FRAME PHASE 2
				// render the frame
				userInterface.startFrame();
				m_renderer.beginSwapchainRenderPass(commandBuffer);
				renderSystem.renderEntities(frameInfo, m_entities);
				userInterface.runExample();
				userInterface.render(commandBuffer);
				m_renderer.endSwapchainRenderPass(commandBuffer);
				m_renderer.endFrame();
				userInterface.endFrame();
			}
		}
		vkDeviceWaitIdle(m_device.device());
	}

	void Engine::addTextureDependency(std::string handle, std::string filePath)
	{
		m_textureDefinitions.insert(std::make_pair(handle, filePath));
	}

	void Engine::addTextureDependency(std::set<std::pair<std::string, std::string>> filePaths)
	{
		m_textureDefinitions.insert(filePaths.begin(), filePaths.end());
	}
	//  Interface end  ----------------------------------

	void Engine::loadTextures()
	{
		for (auto [handle, filePath] : m_textureDefinitions)
		{
			m_textures.emplace(handle, std::make_shared<Texture>(m_device));
			m_textures[handle]->loadFromFile(filePath);
		}
	}

	void Engine::loadEntities()
	{
		Model::VertexData vertexData{};
		vertexData.vertices =
		{
			{{-0.5f, -0.5f}},
			{{ 0.5f, -0.5f}},
			{{-0.5f,  0.5f}},
			{{ 0.5f,  0.5f}}
		};

		vertexData.indices = { 0, 1, 2, 3, 2, 1 };

		//std::vector<Model::Vertex> vertices
		//{
		//	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		//	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		//	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		//};

		auto model = std::make_shared<Model>(m_device, vertexData);
		//auto texture = std::make_shared<Texture>(m_device);
		//texture->loadFromFile("bird.png");

		auto triangle = Entity::createEntity();
		triangle.model = model;
		//triangle.texture = m_textures["bird"];
		//triangle.texture = texture;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2D.translation.x = 0.0f;
		triangle.transform2D.scale = { 1.0f, 1.0f };
		triangle.transform2D.rotation = 0.25f * glm::two_pi<float>();
		m_entities.push_back(std::move(triangle));
	}
} // namespace wrengine