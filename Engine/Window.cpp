#include "Window.h"
namespace wrengine
{
	Window::Window(
		const uint32_t width = 800,
		const uint32_t height = 600
	) :
		m_width{ width },
		m_height{ height }
	{
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW!");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(800, 600, "please save me from this hell", nullptr, nullptr);
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