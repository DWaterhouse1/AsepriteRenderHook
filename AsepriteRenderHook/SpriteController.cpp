#include "SpriteController.h"

void SpriteController::onCreate()
{
	m_transformComponent = &getComponent<wrengine::TransformComponent>();
	m_transformComponent->translation = { 0.0f, 0.0f, 0.5f };
	//m_transformComponent->scale = { 512.0f, 512.0f, 512.0f };
}

void SpriteController::onUpdate(float deltaTime)
{
	//spritePos += (0.5f * deltaTime);
	//spritePos = std::fmod(spritePos, 360.0f);
	//m_transformComponent->translation.x = 100.0f * std::sin(spritePos);
}