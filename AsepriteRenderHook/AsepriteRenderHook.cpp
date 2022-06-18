// AsepriteRenderHook.cpp : Defines the entry point for the application.
//

#include "AsepriteRenderHook.h"

AsepriteRenderHook::AsepriteRenderHook() {}

void AsepriteRenderHook::run()
{
	m_renderer.initImGui();
	m_renderer.mainLoop();
}

void AsepriteRenderHook::mainLoop()
{
	while (!m_renderer.windowShouldClose())
	{
		glfwPollEvents();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();
		m_renderer.drawFrame();
	}
	m_renderer.waitIdle();
}