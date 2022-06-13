#include "Window.h"
namespace wrengine
{
	Window::Window(
		const uint32_t width = 800,
		const uint32_t height = 600,
		std::string name = "GLFW"
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
	}
} // namespace wrengine