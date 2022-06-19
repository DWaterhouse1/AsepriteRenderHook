#pragma once

#include <vulkan/vulkan.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Device.h"
#include "Renderer.h"
#include "Model.h"
#include "Entity.h"

//std
#include <string>
#include <memory>
#include <vector>

namespace wrengine
{
	class Engine
	{
	public:
		Engine(
			const uint32_t width = 800,
			const uint32_t height = 600,
			const std::string& windowName = "wrengine");

		~Engine();

		// should not copy
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		// interface
		bool windowShouldClose();
		void waitIdle();
		void initImGui();
		void mainLoop();

	private:
		// helper functions
		void loadEntities();

		// window params
		uint32_t m_width = 800;
		uint32_t m_height = 600;
		std::string m_windowName = "wrengine";

		// vulkan/glfw structures
		Window m_window{ m_width, m_height, m_windowName };
		Device m_device{ m_window };
		Renderer m_renderer{ m_window, m_device };
		VkPipelineLayout m_pipelineLayout;
		std::vector<Entity> m_entities;
		VkDescriptorPool m_imguiPool;
	};
} // namespace wrengine