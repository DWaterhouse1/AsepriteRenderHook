#pragma once

#include "Device.h"
#include "Model.h"

// std
#include <string>
#include <vector>

namespace wrengine
{
	struct PipelineConfigInfo
	{
		// prevents data loss due to accidental shallow copy of the config info
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline
	{
	public:
		Pipeline(
			Device& device,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& pipelineInfo
		);
		~Pipeline();

		// should not be copied
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t width, uint32_t height);
		void bind(VkCommandBuffer commandBuffer);

	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& pipelineInfo
		);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		Device& m_device;
		VkPipeline m_grahpicsPipeline;
		VkShaderModule m_vertShaderModule;
		VkShaderModule m_fragShaderModule;
	};
} // namespace wrengine