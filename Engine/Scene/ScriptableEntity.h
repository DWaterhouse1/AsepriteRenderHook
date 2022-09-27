#pragma once

#include "Entity.h"

namespace wrengine
{

class ScriptableEntity
{
public:
	virtual ~ScriptableEntity() {}

	template<typename T>
	T& getComponent()
	{
		return m_entity.getComponent<T>();
	}

protected:
	virtual void onCreate() {}
	virtual void onDestroy() {}
	virtual void onUpdate(float deltaTime) {}

private:
	Entity m_entity;
	friend class Scene;
};

} // namepsace wrengine