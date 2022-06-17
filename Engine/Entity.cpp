#include "Entity.h"

namespace wrengine
{
	Entity Entity::createEntity()
	{
		static id_t currentID = 0;
		return Entity(currentID++);
	}

	glm::mat2 Transform2DComponent::mat2()
	{
		const float s = glm::sin(rotation);
		const float c = glm::cos(rotation);
		glm::mat2 rotMat{ {c, s}, {-s, c} };

		glm::mat2 scaleMat{ {scale.x, 0.0f}, {0.0f, scale.y} };
		return rotMat * scaleMat;
	}
}