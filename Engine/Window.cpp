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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, framebufferResizedCallback);
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
	void Window::framebufferResizedCallback(GLFWwindow* window, int width, int height)
	{
		auto recreateWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		recreateWindow->bFramebufferResized = true;
		recreateWindow->m_width = static_cast<uint32_t>(width);
		recreateWindow->m_height = static_cast<uint32_t>(height);
	}
} // namespace wrengine