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
		TransformComponent,
		PointLightComponent>();
	std::shared_ptr<Camera> camera = activeSceneLock->getActiveCamera();

	size_t lightIndex = 0;
	for (auto&& [entity, transform, light] : pointLightView.each())
	{
		glm::vec4 lightPos = glm::vec4{ transform.translation, 0.0f };
		lightPos = camera->getProjection() * lightPos;
		ubo.pointLights[lightIndex].position = lightPos;
		ubo.pointLights[lightIndex].color = light.lightColor;
		lightIndex++;
	}
	ubo.numLights = lightIndex;
}
} // namespace wrengine