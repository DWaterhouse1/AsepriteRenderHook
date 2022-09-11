#pragma once

#include "Scene.h"

#include "entt/entt.hpp"

//std
#include <memory>
#include <optional>

namespace wrengine
{
class Entity
{
public:
	Entity() = default;
	Entity(entt::entity handle, std::shared_ptr<Scene> m_scene);

	// shallow copyable
	Entity(const Entity& other) = default;

	operator bool() const { return m_entityHandle != entt::null; }
	operator entt::entity() const { return m_entityHandle; }

	template<typename T, typename... Args>
	T& addComponent(Args&&... args)
	{
		auto sceneLock = m_scene.lock();
		assert(sceneLock && "cannot add entity components outside scene lifetime!");
		return sceneLock->m_registry.emplace<T>(
			m_entityHandle,
			std::forward<Args>(args)...);

	}

	template<typename T>
	T& getComponent()
	{
		auto sceneLock = m_scene.lock();
		assert(sceneLock && "cannot get entity components outside scene lifetime!");
		return sceneLock->m_registry.get<T>(m_entityHandle);
	}

	template<typename T>
	void removeComponent()
	{
		auto sceneLock = m_scene.lock();
		assert(sceneLock && "cannot remove entity components outside scene lifetime!");
		sceneLock->m_registry.remove<T>(m_entityHandle);
	}

private:
	entt::entity m_entityHandle;
	std::weak_ptr<Scene> m_scene;
};
} // namespace wrengine