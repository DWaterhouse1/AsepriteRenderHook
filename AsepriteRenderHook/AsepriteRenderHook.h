#pragma once

#include "Renderer.h"
#include "Window.h"

// std
#include <iostream>
#include <memory>


class AsepriteRenderHook
{

public:
	AsepriteRenderHook();

	void run();

private:
	void mainLoop();

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	
	wrengine::Renderer m_renderer;
};