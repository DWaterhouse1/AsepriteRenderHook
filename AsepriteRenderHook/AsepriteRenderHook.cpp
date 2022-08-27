// AsepriteRenderHook.cpp : Defines the entry point for the application.
//

#include "AsepriteRenderHook.h"

AsepriteRenderHook::AsepriteRenderHook() {}

void AsepriteRenderHook::run()
{
	initServer();
	initEngine();
	m_engine.run();
}

void AsepriteRenderHook::initServer()
{
	m_server.bindMessageHandler(&messageHandler);
}

void AsepriteRenderHook::initEngine()
{
	m_engine.addTextureDependency(
		{
			{"albedo", "Gemstone_Albedo.png"},
			{"normal", "Gemstone_Normal.png"},
		});
}

void AsepriteRenderHook::messageHandler(WebsocketServer::MessageType message)
{

}