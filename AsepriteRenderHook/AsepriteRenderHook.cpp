// AsepriteRenderHook.cpp : Defines the entry point for the application.
//

#include "AsepriteRenderHook.h"

AsepriteRenderHook::AsepriteRenderHook() {}

void AsepriteRenderHook::run()
{
	m_renderer.initImGui();
	mainLoop();
}

void AsepriteRenderHook::mainLoop()
{
	while (!m_renderer.windowShouldClose())
	{
		glfwPollEvents();
		m_renderer.drawFrame();
	}
	m_renderer.waitIdle();
}