#include "Window.h"

#include <iostream>

namespace wrengine
{
Window::Window(
	const uint32_t width,
	const uint32_t height,
	const std::string& name) :
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
	
	// start hidden
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	m_window = glfwCreateWindow(
		m_width,
		m_height,
		m_windowName.c_str(),
		nullptr,
		nullptr);
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

/**
* 
*/
void Window::show()
{
	glfwShowWindow(m_window);
}

/**
* Initializes window for use with ImGui. Optionally registers ImGui window
* callbacks.
* 
* @param installCallbacks Flag for if the function will register the ImGui
* callbacks.
*/
void Window::windowInitImGui(bool installCallbacks)
{
	ImGui_ImplGlfw_InitForVulkan(m_window, installCallbacks);
}

/**
* Creates Vulkan surface object for use with the window. Will throw on failure
* to create window surface.
* 
* @param instance The Vulkan instance in which the window and surface live.
* @param surface Pointer to the created surface object.
*/
void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(
		instance,
		m_window,
		nullptr,
		surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface");
	}
}

/**
* Callback function to reset the window paramaters on resize.
* @param window Pointer to underlying glfw window.
* @param width New width.
* @param height New height.
*/
void Window::framebufferResizedCallback(
	GLFWwindow* window,
	int width,
	int height)
{
	Window* recreateWindow = reinterpret_cast<Window*>(
		glfwGetWindowUserPointer(window));
	recreateWindow->bFramebufferResized = true;
	recreateWindow->m_width = static_cast<uint32_t>(width);
	recreateWindow->m_height = static_cast<uint32_t>(height);
}
} // namespace wrengine