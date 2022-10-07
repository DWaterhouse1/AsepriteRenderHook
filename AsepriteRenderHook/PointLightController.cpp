#include "PointLightController.h"

#include <cmath>

void PointLightController::onCreate()
{
	m_transformComponent = &getComponent<wrengine::Transform2DComponent>();
	m_transformComponent->position = glm::vec2(0.8f, 0.8f);
}

void PointLightController::onUpdate(float deltaTime)
{
	m_lightAngle += deltaTime * m_rotationSpeed;
	m_lightAngle = std::fmod(m_lightAngle, 360.f);
	float s = std::sin(m_lightAngle);
	float c = std::cos(m_lightAngle);

	//m_transformComponent->position = glm::vec2(s * m_radius, c * m_radius);
}