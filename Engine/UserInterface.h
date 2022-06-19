#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Device.h"
#include "Window.h"

// at some point this class can be static, when descriptor pools are managed by a dedicated class

namespace wrengine
{
	static void check_vk_result(VkResult err) {
		if (err == 0) return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0) abort();
	}

	class UserInterface
	{
	public:
		UserInterface(
			Window& window,
			Device& device,
			VkRenderPass renderPass,
			uint32_t imageCount
		);

		~UserInterface();

		void startFrame();
		void endFrame();

		void render(VkCommandBuffer commandBuffer);

		// example state
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		void runExample();

	private:
		// enable/disable the experimental docking features
		// TODO fix the docking features by ensuring ImGui has a compatible render pass to use
		const bool enableDocking = false;

		Device& m_device;
		VkDescriptorPool m_descriptorPool;
	};
} // namespace wrengine