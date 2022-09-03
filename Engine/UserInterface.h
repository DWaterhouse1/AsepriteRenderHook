#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Device.h"
#include "Window.h"
#include "ElementManager.h"

//std
#include <vector>
#include <memory>

// at some point this class can be static, when descriptor pools are managed by
// a dedicated class

namespace wrengine
{
static void check_vk_result(VkResult err) {
	if (err == 0) return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0) abort();
}

/**
* Simple integration of ImGui
*/
class UserInterface
{
public:
	UserInterface(
		Window& window,
		Device& device,
		VkRenderPass renderPass,
		uint32_t imageCount);

	~UserInterface();

	void startFrame();
	void endFrame();
	void render(VkCommandBuffer commandBuffer);
	std::shared_ptr<ElementManager> getElementManager() { return m_elementManager; }

private:
	// enable/disable the experimental docking features
	// TODO fix the docking features by ensuring ImGui has a compatible render pass to use
	static constexpr bool enableDocking = false;

	Device& m_device;
	VkDescriptorPool m_descriptorPool;
	std::shared_ptr<ElementManager> m_elementManager;
};
} // namespace wrengine