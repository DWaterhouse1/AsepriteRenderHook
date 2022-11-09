#pragma once

#include <vulkan/vulkan.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Device.h"
#include "Pipeline.h"
#include "Model.h"
#include "FrameInfo.h"
#include "Scene/Scene.h"

//std
#include <string>
#include <memory>
#include <vector>

namespace wrengine
{
/**
* Simple system to specify a rendering structure, and configure a corresponding
* pipeline object.
*/
class RenderSystem
{
public:
	RenderSystem(
		Device& device,
		VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout materialSetLayout,
		std::shared_ptr<Scene> activeScene);

	~RenderSystem();

	// should not copy
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	// interface
	void renderEntities(const FrameInfo& frameInfo);
	void updateNormalCoords(glm::vec3 scales);

private:
	// helper functions
	void createPipelineLayout(
		VkDescriptorSetLayout globalSetLayout,
		VkDescriptorSetLayout materialSetLayout);
	void createPipeline(VkRenderPass renderPass);
	void bindQuad(const VkCommandBuffer& frameInfo);
	void drawQuad(const VkCommandBuffer& frameInfo);

	// vulkan/glfw structures
	Device& m_device;
	std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;

	// scene to render
	std::weak_ptr<Scene> m_activeScene;

	// TODO figure out a better system for drawing quads
	std::unique_ptr<Model> m_quadModel;

	// normal coordinates
	glm::vec3 m_normalCoordScales{ 1.0f, -1.0f, 1.0f };
};
} // namespace wrengine