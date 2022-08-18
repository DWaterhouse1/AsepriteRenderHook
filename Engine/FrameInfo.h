#pragma once

#include <vulkan/vulkan.hpp>

namespace wrengine
{
/**
* Struct containing per frame information.
*/
struct FrameInfo
{
	int frameIndex;
	float frameTime;
	VkCommandBuffer commandBuffer;
	VkDescriptorSet globalDescriptorSet;
};
} // namespace wrengine