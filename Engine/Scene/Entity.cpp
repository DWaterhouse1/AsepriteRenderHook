#include "Entity.h"

namespace wrengine
{
Entity::Entity(entt::entity handle, std::shared_ptr<Scene> scene) :
	m_entityHandle{ handle },
	m_scene{ scene }
{

}
} // namespace wrengine