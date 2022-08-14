#pragma once

#include <vulkan/vulkan.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Device.h"
#include "Pipeline.h"
#include "Model.h"
#include "Entity.h"
#include "FrameInfo.h"

//std
#include <string>
#include <memory>
#include <vector>

namespace wrengine
{
	class RenderSystem
	{
	public:
		RenderSystem(
			Device& device,
			VkRenderPass renderPass,
			VkDescriptorSetLayout globalSetLayout);

		~RenderSystem();

		// should not copy
		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;

		// interface
		void renderEntities(
			const FrameInfo& frameInfo,
			std::vector<Entity>& entities);

	private:
		// helper functions
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		// vulkan/glfw structures
		Device& m_device;
		std::unique_ptr<Pipeline> m_pipeline;
		VkPipelineLayout m_pipelineLayout;
	};
} // namespace wrengine