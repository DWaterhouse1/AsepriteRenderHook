#pragma once

#include <vulkan/vulkan.hpp>

namespace wrengine
{
struct FrameInfo
{
	int frameIndex;
	float frameTime;
	VkCommandBuffer commandBuffer;
	VkDescriptorSet globalDescriptorSet;
};
} // namespace wrengine