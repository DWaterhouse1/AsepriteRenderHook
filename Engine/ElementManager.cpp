#include "ElementManager.h"

namespace wrengine
{

ElementManager::~ElementManager()
{
	for (auto& element : m_elementStack)
	{
		element->onDetatch();
	}
	m_elementStack.clear();
}

/**
* Takes an existing UIElement via base pointer and adds it to the stack. Calls
* onAttach for the element added.
*
* @param element Pointer to element to be added to the stack.
*/
void ElementManager::pushElement(const std::shared_ptr<UIElement>& element)
{
	m_elementStack.emplace_back(element);
	element->onAttach();
}

/**
* Calls tick on each attached ui element.
*
* @param deltaTime The elapsed time since the last frame.
*/
void ElementManager::tickElements(float deltaTime)
{
	for (auto& element : m_elementStack)
	{
		element->tick(deltaTime);
	}
}

/**
* Calls the onUIRender function in every attached UI element. These functions
* contain calls to ImGui that define/update the elements ImGui state.
*/
void ElementManager::runElements()
{
	for (auto& element : m_elementStack)
	{
		element->onUIRender();
	}
}
} // namespace wrengine