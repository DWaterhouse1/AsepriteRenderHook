// AsepriteRenderHook.cpp : Defines the entry point for the application.
//

#include "AsepriteRenderHook.h"

AsepriteRenderHook::AsepriteRenderHook() : m_window(WIDTH, HEIGHT, "Aseprite Render Hook") {}

void AsepriteRenderHook::run()
{
	mainLoop();
}

void AsepriteRenderHook::mainLoop()
{
	while (!m_window.shouldClose())
	{
		glfwPollEvents();
	}
}