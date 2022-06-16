#pragma once

#include <vulkan/vulkan.hpp>

#include "Pipeline.h"
#include "Swapchain.h"
#include "Model.h"

//std
#include <string>
#include <memory>
#include <vector>

namespace wrengine
{
	class Renderer
	{
	public:
		Renderer(
			const uint32_t width = 800,
			const uint32_t height = 600,
			const std::string& windowName = "wrengine");

		~Renderer();

		// should not copy
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		// interface
		bool windowShouldClose();
		void drawFrame();
		void waitIdle();

	private:
		// helper functions
		void loadModels();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();

		// window params
		uint32_t m_width = 800;
		uint32_t m_height = 600;
		std::string m_windowName = "wrengine";

		// vulkan/glfw structures
		Window m_window{ m_width, m_height, m_windowName };
		Device m_device{ m_window };
		Swapchain m_swapchain{ m_device, m_window.getExtent()};
		std::unique_ptr<Pipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		std::unique_ptr<Model> m_model;
		std::vector<VkCommandBuffer> m_commandBuffers;
	};
} // namespace wrengine