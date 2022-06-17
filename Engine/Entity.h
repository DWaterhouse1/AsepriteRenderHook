#pragma once

#include "Model.h"

// std
#include <memory>

namespace wrengine
{
	struct Transform2DComponent
	{
		glm::vec2 translation{};
		glm::vec2 scale{ 1.0f, 1.0f };
		float rotation;
		
		glm::mat2 mat2();
	};

	class Entity
	{
	public:
		using id_t = unsigned int;

		static Entity createEntity();
		id_t getID() const { return m_id; }

		// no duplicate entities
		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;

		// movable
		Entity(Entity&&) = default;
		Entity& operator=(Entity&&) = default;

		std::shared_ptr<Model> model{};
		glm::vec3 color{};
		Transform2DComponent transform2D{};

	private:
		Entity(id_t entityID) : m_id{ entityID } {}

		id_t m_id;
	};
}