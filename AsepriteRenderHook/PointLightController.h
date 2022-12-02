#pragma once

#include "Wrengine.h"

class PointLightController : public wrengine::ScriptableEntity
{
public:
	virtual void onCreate() override;
	virtual void onUpdate(float deltaTime) override;

	wrengine::TransformComponent* m_transformComponent = nullptr;
	float m_lightAngle = 0.0f;
	float m_rotationSpeed = 0.5f;
	float m_radius = 2.0f;
};