#pragma once

#include "Websocket.h"

// wrengine
#include "Engine.h"
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
	void initServer();
	void initEngine();

	void messageHandler(WebsocketServer::MessageType message);

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	const uint16_t PORT = 30001;
	
	WebsocketServer m_server{ PORT };
	wrengine::Engine m_engine{ WIDTH, HEIGHT, "Aseprite Render Hook" };
};