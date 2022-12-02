#include "PointLightController.h"

#include <cmath>

void PointLightController::onCreate()
{
	m_transformComponent = &getComponent<wrengine::TransformComponent>();
	m_transformComponent->scale = glm::vec3(64.0f);
	m_transformComponent->translation = glm::vec3(200.0f, -200.0f, 0.5f);
}

void PointLightController::onUpdate(float deltaTime)
{
	m_lightAngle += deltaTime * m_rotationSpeed;
	m_lightAngle = std::fmod(m_lightAngle, 360.f);
	float s = std::sin(m_lightAngle);
	float c = std::cos(m_lightAngle);

	m_transformComponent->translation = 100.0f * glm::vec3(1.5f * s * m_radius, 0.1f * c * m_radius, 0.0f);
}