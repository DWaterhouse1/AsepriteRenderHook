#include "Window.h"

#include <iostream>

namespace wrengine
{
	Window::Window(
		const uint32_t width,
		const uint32_t height,
		const std::string& name
	) :
		m_width{ width },
		m_height{ height },
		m_windowName{ name }

	{
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW!");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
		if (!m_window)
		{
			throw std::runtime_error("Failed to create GLFW window!");
		}
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface");
		}
	}
} // namespace wrengine