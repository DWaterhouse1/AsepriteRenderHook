#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <stdexcept>
#include <string>

namespace wrengine
{
	class Window
	{
	public:
		Window(
			const uint32_t width,
			const uint32_t height
			//std::string name
		);
		~Window();

		// class should not be copied
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(m_window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		const uint32_t m_width;
		const uint32_t m_height;
		//std::string m_windowName;
		GLFWwindow* m_window = nullptr;
	};
} // namespace wrengine