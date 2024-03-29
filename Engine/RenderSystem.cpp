#include "RenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <cmath>

/**
* Simple struct for push constants. Contains transform, offset and color data.
* w element of normalsTransform is padding.
*/
struct PushConstantData
{
	glm::mat4 model{ 1.0f };
	uint32_t shaderConfig = 0;
	alignas(16) glm::vec4 normalsTransform = { 1.0f, -1.0f, 1.0f, 0.0f };
};

namespace wrengine
{
RenderSystem::RenderSystem(
	Device& device,
	VkRenderPass renderPass,
	VkDescriptorSetLayout globalSetLayout,
	VkDescriptorSetLayout materialSetLayout,
	std::shared_ptr<Scene> activeScene) :
	m_device{ device },
	m_activeScene{ activeScene }
{
	createPipelineLayout(globalSetLayout, materialSetLayout);
	createPipeline(renderPass);

	// initialise quad model data 
	// TODO do this somewhere else, maybe constants header?
	Model::VertexData vertexData{};
	vertexData.vertices =
	{
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};

	vertexData.indices = { 0, 1, 2, 2, 3, 0 };
	m_quadModel = std::make_unique<Model>(m_device, vertexData);
}

RenderSystem::~RenderSystem()
{
	vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void RenderSystem::updateNormalCoords(glm::vec3 scales)
{
	m_normalCoordScales = scales;
}

/**
* Creates a pipeline layout owned by the render system acording to supplied
* descriptor set layouts.
* 
* @param globalSetLayout The current global descriptor set layout.
*/
void RenderSystem::createPipelineLayout(
	VkDescriptorSetLayout globalSetLayout,
	VkDescriptorSetLayout materialSetLayout)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags =
		VK_SHADER_STAGE_VERTEX_BIT |
		VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.size = sizeof(PushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayout{
		globalSetLayout,
		materialSetLayout
	};

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
	createInfo.pSetLayouts = descriptorSetLayout.data();
	createInfo.pushConstantRangeCount = 1;
	createInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(
		m_device.device(),
		&createInfo,
		nullptr,
		&m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

/**
* Creates a pipeline object owned by the render system, using hardcoded shader
* file paths.
* 
* @param renderPass The render pass object to use in pipeline configuration.
*/
void RenderSystem::createPipeline(VkRenderPass renderPass)
{
	assert(m_pipelineLayout != nullptr &&
		"cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
	pipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_pipelineLayout;
	m_pipeline = std::make_unique<Pipeline>(
		m_device,
		"shaders/simple_shader.vert.spv",
		"shaders/simple_shader.frag.spv",
		pipelineConfig);
}

/**
* Submits command to render the provided Entity objects.
* 
* @param frameInfo Structure describing relevent current frame information.
* @param entities Vector of Entity objects to render.
*/
void RenderSystem::renderEntities(const FrameInfo& frameInfo)
{
	// TODO handle this more precisely than just throwing an exception.
	// log and return?
	auto activeSceneLock = m_activeScene.lock();
	if (!activeSceneLock)
	{
		throw std::runtime_error("can't render entities without active scene!");
	}

	m_pipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(
		frameInfo.commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0, 1,
		&frameInfo.globalDescriptorSet,
		0, nullptr);
	
	auto renderView = activeSceneLock->getAllEntitiesWith<
		TransformComponent,
		SpriteRenderComponent>();

	std::shared_ptr<Camera> camera = activeSceneLock->getActiveCamera();
	glm::mat4 projectionView = camera->getProjection() * camera->getView();

	for (auto&& [entity, transform, render] : renderView.each())
	{
		PushConstantData push{};

		glm::mat4 model = glm::translate(glm::mat4{ 1.0f }, transform.translation);
		model = glm::scale(model, transform.scale);
		model = glm::rotate(model, transform.rotation.y, glm::vec3{ 0.0f, 1.0f, 0.0f });
		model = glm::rotate(model, transform.rotation.x, glm::vec3{ 1.0f, 0.0f, 0.0f });
		model = glm::rotate(model, transform.rotation.z, glm::vec3{ 0.0f, 0.0f, 1.0f });

		push.model = model;
		push.shaderConfig = static_cast<uint32_t>(render.material.shaderConfig);
		push.normalsTransform = glm::vec4{ m_normalCoordScales, 0.0f };

		vkCmdPushConstants(
			frameInfo.commandBuffer,
			m_pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			1, 1,
			&render.material.materialDescriptor,
			0, nullptr);

		bindQuad(frameInfo.commandBuffer);
		drawQuad(frameInfo.commandBuffer);
	}
}

/**
* Binds the pipeline ready to draw standard quad model.
*
* @param commandBuffer The command buffer into which bind commands will be
* recorded. 
*/
void RenderSystem::bindQuad(const VkCommandBuffer& commandBuffer)
{
	m_quadModel->bind(commandBuffer);
}

/**
* Draws current pipeline state using standard quad model.
* 
* @param commandBuffer The command buffer into which draw commands will be
* recorded.
*/
void RenderSystem::drawQuad(const VkCommandBuffer& commandBuffer)
{
	m_quadModel->draw(commandBuffer);
}
} // namespace wrengine