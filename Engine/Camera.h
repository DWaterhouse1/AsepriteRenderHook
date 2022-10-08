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

	const glm::mat4& getProjection() const { return m_projection; }
private:
	glm::mat4 m_projection;
};

} // namespace wrengine