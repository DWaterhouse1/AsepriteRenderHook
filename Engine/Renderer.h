#pragma once

#include <vulkan/vulkan.hpp>

#include "Pipeline.h"

namespace wrengine
{
	class Renderer
	{
	public:
		Renderer();

	private:
		Pipeline pipeline{ "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv" };
	};
} // namespace wrengine