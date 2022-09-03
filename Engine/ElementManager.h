#pragma once

#include "InterfaceElement.h"

// std
#include <vector>
#include <memory>

namespace wrengine
{

class ElementManager
{
public:
	ElementManager() {}
	~ElementManager();

	// not copyable
	ElementManager(const ElementManager&) = delete;
	ElementManager& operator=(const ElementManager&) = delete;

	// default move of a vector of shared ptr is ok
	ElementManager(ElementManager&&) = default;
	ElementManager& operator=(ElementManager&&) = default;

	void pushElement(const std::shared_ptr<UIElement>& element);

	void tickElements(float deltaTime);
	void runElements();

	/**
	* Creates and adds a new UI element, calling onAttach. Supplied element must
	* be a subclass of UIElement.
	*/
	template<typename T>
	void pushElement()
	{
		static_assert(
			std::is_base_of<UIElement, T>::value,
			"Pushed type is not subclass of UIElement!");
		std::shared_ptr<T> element = std::make_shared<T>();
		element->onAttach();
		m_elementStack.push_back(element);
		//m_elementStack.push_back(std::make_shared<T>())->onAttach();
	}

private:
	std::vector<std::shared_ptr<UIElement>> m_elementStack{};
};
} // namespace wrengine