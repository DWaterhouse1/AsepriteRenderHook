#pragma once

#include <vulkan/vulkan.hpp>

#include "Swapchain.h"
#include "Model.h"

//std
#include <string>
#include <memory>
#include <vector>
#include <cassert>

namespace wrengine
{
/**
* Abstraction over rendering functions. Owns and operates the Swapchain and
* command buffers. Responsible for frame drawing functions.
*/
class Renderer
{
public:
	Renderer(Window& window, Device& device);

	~Renderer();

	// should not copy
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	// interface
	VkCommandBuffer beginFrame();
	void endFrame();
	void beginSwapchainRenderPass(VkCommandBuffer commandBuffer);
	void endSwapchainRenderPass(VkCommandBuffer commandBuffer);
	bool isFrameInProgress() const { return m_isFrameStarted; }
	VkCommandBuffer getCurrentCommandBuffer() const;
	VkRenderPass getSwapchainRenderPass() const;
	void waitIdle();
	int getFrameIndex() const;
	size_t getImageCount() { return m_swapchain->imageCount(); }

private:
	// helper functions
	void createCommandBuffers();
	void freeCommandBuffers();
	void recreateSwapchain();

	uint32_t m_currentImageIndex;
	int m_currentFrameIndex = 0;
	bool m_isFrameStarted = false;

	// window params
	uint32_t m_width = 800;
	uint32_t m_height = 600;
	std::string m_windowName = "wrengine";

	// vulkan/glfw structures
	Window& m_window;
	Device& m_device;
	std::unique_ptr<Swapchain> m_swapchain;
	std::vector<VkCommandBuffer> m_commandBuffers;;
};
} // namespace wrengine