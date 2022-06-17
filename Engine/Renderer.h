#pragma once

#include <vulkan/vulkan.hpp>

#include "Pipeline.h"
#include "Swapchain.h"
#include "Model.h"
#include "Entity.h"

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
		void loadEntities();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapchain();
		void recordCommandBuffer(uint32_t imageIndex);
		void renderEntities(VkCommandBuffer commandBuffer);

		// window params
		uint32_t m_width = 800;
		uint32_t m_height = 600;
		std::string m_windowName = "wrengine";

		// vulkan/glfw structures
		Window m_window{ m_width, m_height, m_windowName };
		Device m_device{ m_window };
		std::unique_ptr<Swapchain> m_swapchain;
		std::unique_ptr<Pipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<Entity> m_entities;
		std::vector<VkCommandBuffer> m_commandBuffers;
	};
} // namespace wrengine