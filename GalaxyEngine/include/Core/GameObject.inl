#pragma once
#include "Core/GameObject.h"

template<typename T>
inline std::weak_ptr<T> Core::GameObject::AddComponent()
{
	if (!std::is_base_of<Component::BaseComponent, T>::value) {
		PrintError("Incorrect Type for component");
		return;
	}

	std::shared_ptr<T> component = std::make_shared<T>();
	AddComponent(component);
}

template<typename T>
inline void Core::GameObject::AddComponent(std::shared_ptr<T> component)
{
	if (!std::is_base_of<Component::BaseComponent, T>::value) {
		PrintError("Incorrect Type for component");
		return;
	}
	component->gameObject = weak_from_this();
	m_components.push_back(component);
}