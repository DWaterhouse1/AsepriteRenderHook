#pragma once

#include "Components.h"

#include "entt/entt.hpp"

//std
#include <memory>
#include <string>

namespace wrengine
{
class Entity;

class Scene : public std::enable_shared_from_this<Scene>
{
public:
	Scene();
	~Scene();

	Entity createEntity(const std::string& name = "");
	void destroyEntity(Entity entity);

	template<typename... Components>
	auto getAllEntitiesWith()
	{
		return m_registry.view<Components...>();
	}

	std::shared_ptr<Camera> getActiveCamera() { return m_activeCamera; }

	void onSceneStart();
	void onUpdate(float deltaTime);

private:
	entt::registry m_registry;

	friend class Entity;

	std::shared_ptr<Camera> m_activeCamera{};
};

} // namespace wrengine