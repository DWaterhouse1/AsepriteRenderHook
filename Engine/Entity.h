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
struct Transform2DComponent
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
class Entity
{
public:
	using id_t = unsigned int;

	static Entity createEntity();
	id_t getID() const { return m_id; }

	// no duplicate entities
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	// member wise move is sufficient
	Entity(Entity&&) = default;
	Entity& operator=(Entity&&) = default;

	std::shared_ptr<Model> model{};
	std::shared_ptr<Texture> texture{};
	glm::vec3 color{};
	Transform2DComponent transform2D{};

private:
	Entity(id_t entityID) : m_id{ entityID } {}

	id_t m_id;
};
}