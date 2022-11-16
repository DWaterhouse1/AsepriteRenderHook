#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace wrengine
{

class Camera
{
public:
	Camera() = default;
	Camera(const glm::mat4& projection) : m_projection{ projection } {}

	void setOrthographicProjection(
		float leftPlane,
		float rightPlane,
		float topPlane,
		float bottomPlane,
		float nearPlane,
		float farPlane);

	void setPerspectiveProjection(
		float fieldOfViewY,
		float aspect,
		float nearPlane,
		float farPlane);

	void setViewDirection(
		glm::vec3 position,
		glm::vec3 direction,
		glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });
	void setViewTarget(
		glm::vec3 position,
		glm::vec3 target,
		glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });
	void setViewEulerYXZ(
		glm::vec3 position,
		glm::vec3 rotation);

	const glm::mat4& getProjection()	const { return m_projection; }
	const glm::mat4& getInverseProjection() const { return m_inverseProjection; }
	const glm::mat4& getView()				const { return m_view; }
private:
	void computeInverseProjection();
	glm::mat4 m_projection{ 1.0f };
	glm::mat4 m_inverseProjection{ 1.0f };
	glm::mat4 m_view{ 1.0f };
};

} // namespace wrengine