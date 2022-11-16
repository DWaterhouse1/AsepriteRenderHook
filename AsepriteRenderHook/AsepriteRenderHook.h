﻿#pragma once

#include "Websocket.h"
#include "DemoWindow.h"

// wrengine
#include "Wrengine.h"

// std
#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>

class MovingSprite : public wrengine::ScriptableEntity
{
public:
	void onCreate() override
	{
		m_transformComponent = &getComponent<wrengine::TransformComponent>();
		m_transformComponent->translation = { 0.0f, 0.0f, 0.5f };
		m_transformComponent->scale = { 512.0f, 512.0f, 512.0f };
	}

	void onUpdate(float deltaTime) override
	{
		spritePos += (0.5f * deltaTime);
		spritePos = std::fmod(spritePos, 360.0f);
		m_transformComponent->translation.x = 100.0f * std::sin(spritePos);
	}

private:
	wrengine::TransformComponent* m_transformComponent = nullptr;

	float spritePos = 0.0f;
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
	std::mutex m_conditionMutex;
	std::condition_variable m_initCondition;
	std::vector<uint8_t> m_diffuseData;
	std::vector<uint8_t> m_normalData;
	int m_spriteWidth = 0;
	int m_spriteHeight = 0;
};