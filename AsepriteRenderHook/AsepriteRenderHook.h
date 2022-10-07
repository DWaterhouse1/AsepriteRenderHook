#pragma once

#include "Websocket.h"
#include "DemoWindow.h"

// wrengine
#include "Wrengine.h"

// std
#include <iostream>
#include <memory>
#include <vector>

class MovingSprite : public wrengine::ScriptableEntity
{
public:
	void onCreate() override
	{
		m_transformComponent = &getComponent<wrengine::Transform2DComponent>();
	}

	void onUpdate(float deltaTime) override
	{
		spriteAngle += (0.5f * deltaTime);
		spriteAngle = std::fmod(spriteAngle, 360.0f);
		m_transformComponent->position.x = std::sin(spriteAngle);
	}

private:
	wrengine::Transform2DComponent* m_transformComponent;

	float spriteAngle = 0.0f;
};

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
	std::unique_ptr<wrengine::Engine> m_engine;

	// texture data
	std::vector<uint8_t> m_diffuseData;
	std::vector<uint8_t> m_normalData;
};