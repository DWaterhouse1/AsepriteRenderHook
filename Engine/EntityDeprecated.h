#pragma once

#include "Model.h"
#include "Texture.h"

// std
#include <memory>

namespace wrengine
{
/**
* Component structure representing the spatial transform of a 2d object,
* including translation scale and rotation components.
*/
struct Transform2DComponentDeprecated
{
	glm::vec2 translation{};
	glm::vec2 scale{ 1.0f, 1.0f };
	float rotation;
	
	glm::mat2 mat2();
};

/**
* Fundamental renderable class. Entities are created using the static
* createEntity() function in order to ensure each entity has a unique id_t.
*/
class EntityDeprecated
{
public:
	using id_t = unsigned int;

	static EntityDeprecated createEntity();
	id_t getID() const { return m_id; }

	// no duplicate entities
	EntityDeprecated(const EntityDeprecated&) = delete;
	EntityDeprecated& operator=(const EntityDeprecated&) = delete;

	// member wise move is sufficient
	EntityDeprecated(EntityDeprecated&&) = default;
	EntityDeprecated& operator=(EntityDeprecated&&) = default;

	std::shared_ptr<Model> model{};
	std::shared_ptr<Texture> texture{};
	glm::vec3 color{};
	Transform2DComponentDeprecated transform2D{};

private:
	EntityDeprecated(id_t entityID) : m_id{ entityID } {}

	id_t m_id;
};
}