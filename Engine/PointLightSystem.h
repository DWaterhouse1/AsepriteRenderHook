#pragma once

#include "FrameInfo.h"

#include "Scene/Scene.h"

//std
#include <memory>

namespace wrengine
{
class PointLightSystem
{
public:
	PointLightSystem(std::shared_ptr<Scene> scene);

	void update(GlobalUbo& ubo);

private:
	std::weak_ptr<Scene> m_activeScene;
};
} // namespace wrengine