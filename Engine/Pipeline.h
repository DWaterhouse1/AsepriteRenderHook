#pragma once

#include "Device.h"

// std
#include <string>
#include <vector>

namespace wrengine
{
	struct PipelineConfigInfo {};
	class Pipeline
	{
	public:
		Pipeline(
			Device& device,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& pipelineInfo
		);
		~Pipeline() {}

		// should not be copied
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

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