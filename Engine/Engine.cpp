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
	}

	// Interface start ----------------------------------
	void Engine::run()
	{
		RenderSystem renderSystem{ m_device, m_renderer.getSwapchainRenderPass() };

		UserInterface userInterface{
			m_window,
			m_device,
			m_renderer.getSwapchainRenderPass(),
			static_cast<uint32_t>(m_renderer.getImageCount())
		};

		while (!m_window.shouldClose())
		{
			glfwPollEvents();
			if (auto commandBuffer = m_renderer.beginFrame())
			{
				userInterface.startFrame();
				m_renderer.beginSwapchainRenderPass(commandBuffer);
				renderSystem.renderEntities(commandBuffer, m_entities);
				userInterface.runExample();
				userInterface.render(commandBuffer);
				m_renderer.endSwapchainRenderPass(commandBuffer);
				m_renderer.endFrame();
				userInterface.endFrame();
			}
		}
		vkDeviceWaitIdle(m_device.device());
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