#include "Scene.h"
#include "Entity.h"

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
} // namespace wrengine