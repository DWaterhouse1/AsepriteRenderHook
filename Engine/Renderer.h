#pragma once

#include <vulkan/vulkan.hpp>

#include "Pipeline.h"

//std
#include <string>

namespace wrengine
{
	class Renderer
	{
	public:
		Renderer() {}

		bool windowShouldClose();

	private:
		uint32_t m_width = 800;
		uint32_t m_height = 600;
		std::string m_windowName = "wrengine";

		Window m_window{ m_width, m_height, m_windowName };
		Device m_device{ m_window };
		Pipeline m_pipeline{
			m_device,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			Pipeline::defaultPipelineConfigInfo(m_width, m_height)};
	};
} // namespace wrengine