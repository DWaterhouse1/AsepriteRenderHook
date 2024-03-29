#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

// std
#include <stdexcept>
#include <string>
#include <iostream>

namespace wrengine
{
/**
* Simple abstraction over glfw window. Construction will throw a runtime error
* on failure to initialize glfw or to create glfw window.
*/
class Window
{
public:
	Window(
		const uint32_t width = 800,
		const uint32_t height = 600,
		const std::string& name = "GLFW window");
	~Window();

	// class should not be copied
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	void show();
	bool shouldClose() { return glfwWindowShouldClose(m_window); }
	bool wasResized() { return bFramebufferResized; }
	void resetWindowResizeFlag() { bFramebufferResized = false; }
	VkExtent2D getExtent() { return { m_width, m_height }; }
	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	void windowInitImGui(bool installCallbacks);
	GLFWwindow* getGlfwWindow() { return m_window; }

private:
	static void framebufferResizedCallback(GLFWwindow* window, int width, int height);
	uint32_t m_width;
	uint32_t m_height;
	std::string m_windowName;
	GLFWwindow* m_window = nullptr;
	bool bFramebufferResized = false;
};
} // namespace wrengine