#include "Camera.h"

// std
#include <cassert>
#include <limits>

namespace wrengine
{
void Camera::setOrthographicProjection(
	float leftPlane,
	float rightPlane,
	float topPlane,
	float bottomPlane,
	float nearPlane,
	float farPlane)
{
	m_projection = glm::mat4{ 1.0f };
	m_projection[0][0] = 2.0f / (rightPlane - leftPlane);
	m_projection[1][1] = 2.0f / (bottomPlane - topPlane);
	m_projection[2][2] = 1.0f / (farPlane - nearPlane);
	m_projection[3][0] = -(rightPlane + leftPlane) / (rightPlane - leftPlane);
	m_projection[3][1] = -(bottomPlane + topPlane) / (bottomPlane - topPlane);
	m_projection[3][2] = -nearPlane / (farPlane - nearPlane);
}

void Camera::setPerspectiveProjection(
	float fieldOfViewY,
	float aspect,
	float nearPlane,
	float farPlane)
{
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
	const float tanFovHalves = tan(fieldOfViewY / 2.0f);
	m_projection = glm::mat4{ 1.0f };
	m_projection[0][0] = 1.0f / (aspect * tanFovHalves);
	m_projection[1][1] = 1.0f / (tanFovHalves);
	m_projection[2][2] = farPlane / (farPlane - nearPlane);
	m_projection[2][3] = 1.0f;
	m_projection[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
}

void Camera::setViewDirection(
	glm::vec3 position,
	glm::vec3 direction,
	glm::vec3 up)
{
	assert(glm::abs(direction.x - std::numeric_limits<float>::epsilon()) > 0.0f);
	assert(glm::abs(direction.y - std::numeric_limits<float>::epsilon()) > 0.0f);
	assert(glm::abs(direction.z - std::numeric_limits<float>::epsilon()) > 0.0f);

	// {u, v, w} forms an orthonormal basis of R3
	const glm::vec3 w{ glm::normalize(direction) };
	const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
	const glm::vec3 v{ glm::cross(w, u) };

	// simple basis transform mat
	m_view = glm::mat4{ 1.f };
	m_view[0][0] = u.x;
	m_view[1][0] = u.y;
	m_view[2][0] = u.z;
	m_view[0][1] = v.x;
	m_view[1][1] = v.y;
	m_view[2][1] = v.z;
	m_view[0][2] = w.x;
	m_view[1][2] = w.y;
	m_view[2][2] = w.z;
	m_view[3][0] = -glm::dot(u, position);
	m_view[3][1] = -glm::dot(v, position);
	m_view[3][2] = -glm::dot(w, position);
}

void Camera::setViewTarget(
	glm::vec3 position,
	glm::vec3 target,
	glm::vec3 up)
{
	setViewDirection(position, target - position, up);
}

void Camera::setViewEulerYXZ(
	glm::vec3 position,
	glm::vec3 rotation)
{
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
	m_view = glm::mat4{ 1.f };
	m_view[0][0] = u.x;
	m_view[1][0] = u.y;
	m_view[2][0] = u.z;
	m_view[0][1] = v.x;
	m_view[1][1] = v.y;
	m_view[2][1] = v.z;
	m_view[0][2] = w.x;
	m_view[1][2] = w.y;
	m_view[2][2] = w.z;
	m_view[3][0] = -glm::dot(u, position);
	m_view[3][1] = -glm::dot(v, position);
	m_view[3][2] = -glm::dot(w, position);
}
} // namespace wrengine