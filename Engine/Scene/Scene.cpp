#include "Scene.h"
#include "Entity.h"
#include "ScriptableEntity.h"

namespace wrengine
{
Scene::Scene()
{
}

Scene::~Scene()
{
}

Entity Scene::createEntity(const std::string& name)
{
	Entity entity = { m_registry.create(), shared_from_this() };
	auto& tagComponent = entity.addComponent<TagComponent>();
	name.empty() ? "Entity" : name;
	return entity;
}

void Scene::destroyEntity(Entity entity)
{
	m_registry.destroy(entity);
}

void Scene::onSceneStart()
{
	for (auto& [entity, script] : m_registry.view<ScriptComponent>().each())
	{
		if (!script.instance)
		{
			script.instantiate();
			script.instance->m_entity = Entity{ entity, shared_from_this() };
			script.instance->onCreate();
		}
	}
}

void Scene::onUpdate(float deltaTime)
{

	// update scripts
	for (auto& [entity, script] : m_registry.view<ScriptComponent>().each())
	{
		script.instance->onUpdate(deltaTime);
	}
}
} // namespace wrengine