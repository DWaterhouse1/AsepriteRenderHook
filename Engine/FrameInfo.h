#pragma once

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace wrengine
{
constexpr size_t MAX_LIGHTS = 10;

/**
* Struct describing point light data.
*/
struct PointLight
{
	glm::vec4 position{}; // ignore w
	glm::vec4 color{}; // xyzw = R G B Intensity
};

/**
* Struct defining the layout for the globally accessible uniform data.
*/
struct GlobalUbo
{
	glm::vec4 ambientLight{ 1.0f, 1.0f, 1.0f, 0.02f };
	PointLight pointLights[10];
	uint64_t numLights;
};

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