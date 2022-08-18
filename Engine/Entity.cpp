#include "Entity.h"

namespace wrengine
{
/**
* Creates a new entity at the next id_t value.
* 
* @return The created Entity
*/
Entity Entity::createEntity()
{
	static id_t currentID = 0;
	return Entity(currentID++);
}

/**
* Generates a 2x2 matrix representing the 2D transform of the component.
* 
* @return The 2x2 matrix transform.
*/
glm::mat2 Transform2DComponent::mat2()
{
	const float s = glm::sin(rotation);
	const float c = glm::cos(rotation);
	glm::mat2 rotMat{ {c, s}, {-s, c} };

	glm::mat2 scaleMat{ {scale.x, 0.0f}, {0.0f, scale.y} };
	return rotMat * scaleMat;
}
}