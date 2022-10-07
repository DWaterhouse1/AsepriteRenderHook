#include "PointLightSystem.h"

namespace wrengine
{
PointLightSystem::PointLightSystem(std::shared_ptr<Scene> scene) :
	m_activeScene{ scene }
{

}

void PointLightSystem::update(GlobalUbo& ubo)
{
	auto activeSceneLock = m_activeScene.lock();
	if (!activeSceneLock)
	{
		throw std::runtime_error("can't render entities without active scene!");
	}

	auto pointLightView = activeSceneLock->getAllEntitiesWith<
		Transform2DComponent,
		PointLightComponent>();

	size_t lightIndex = 0;
	for (auto&& [entity, transform, light] : pointLightView.each())
	{
		ubo.pointLights[lightIndex].position = glm::vec4{ transform.position, 0.0f, 0.0f };
		ubo.pointLights[lightIndex].color = light.lightColor;
		lightIndex++;
	}
	ubo.numLights = lightIndex;
}
} // namespace wrengine