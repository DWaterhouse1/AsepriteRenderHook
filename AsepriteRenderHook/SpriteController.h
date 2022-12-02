#pragma once

#include "Wrengine.h"

class SpriteController : public wrengine::ScriptableEntity
{
	void onCreate() override;
	void onUpdate(float deltaTime) override;

private:
	wrengine::TransformComponent* m_transformComponent;

	float spritePos = 0.0f;
};