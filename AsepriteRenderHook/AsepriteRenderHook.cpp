// AsepriteRenderHook.cpp : Defines the entry point for the application.
//

#include "AsepriteRenderHook.h"

AsepriteRenderHook::AsepriteRenderHook() {}

void AsepriteRenderHook::run()
{
	mainLoop();
}

void AsepriteRenderHook::mainLoop()
{
	while (!m_renderer.windowShouldClose())
	{
		glfwPollEvents();
	}
}