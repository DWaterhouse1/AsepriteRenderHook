#include "RenderSystem.h"

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
	glm::mat2 transform{ 1.0f };
	glm::vec2 offset;
	alignas(16) glm::vec3 color;
};

namespace wrengine
{
	RenderSystem::RenderSystem(Device& device, VkRenderPass renderPass) : m_device{ device }
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	RenderSystem::~RenderSystem()
	{
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}

	void RenderSystem::createPipelineLayout()
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

	void RenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(m_pipelineLayout != nullptr && "cannot create pipeline before pipeline layout!");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pipeline = std::make_unique<Pipeline>(
			m_device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
			);
	}

	void RenderSystem::renderEntities(VkCommandBuffer commandBuffer, std::vector<Entity>& entities)
	{
		m_pipeline->bind(commandBuffer);
		for (auto& entity : entities)
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
} // namespace wrengine