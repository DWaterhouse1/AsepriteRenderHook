#pragma once

#include "Websocket.h"
#include "DemoWindow.h"

// wrengine
#include "Engine.h"
#include "Window.h"

// std
#include <iostream>
#include <memory>
#include <vector>

class AsepriteRenderHook
{

public:
	AsepriteRenderHook();

	void run();

private:
	void initServer();
	void initEngine();

	void messageHandler(WebsocketServer::MessageType message);

	// image dimensions
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	// port that the server will listen on by default
	const uint16_t PORT = 30001;
	
	WebsocketServer m_server{ PORT };
	wrengine::Engine m_engine{ WIDTH, HEIGHT, "Aseprite Render Hook" };

	// texture data
	std::vector<uint8_t> m_diffuseData;
	std::vector<uint8_t> m_normalData;
};