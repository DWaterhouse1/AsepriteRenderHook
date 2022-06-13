#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <stdexcept>
#include <string>

class ArhWindow
{
public:
	ArhWindow(
		const uint32_t width,
		const uint32_t height,
		std::string name
	);
	~ArhWindow();

	// class should not be copied
	ArhWindow(const ArhWindow&) = delete;
	ArhWindow& operator=(const ArhWindow&) = delete;

	bool shouldClose() { return glfwWindowShouldClose(m_window); }
	void createWindowSurface(VkInstance instance, VkSurfaceKHR surface);

private:
	const uint32_t m_width;
	const uint32_t m_height;
	std::string m_windowName;
	GLFWwindow* m_window;
};
