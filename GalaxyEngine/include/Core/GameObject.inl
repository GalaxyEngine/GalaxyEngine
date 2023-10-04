#pragma once
#include "Core/GameObject.h"

std::string Core::GameObject::GetName() const
{
	return m_name;
}


uint64_t Core::GameObject::GetIndex() const
{
	return m_id;
}

void Core::GameObject::SetName(std::string val)
{
	m_name = val;
}

template<typename T>
inline std::weak_ptr<T> Core::GameObject::AddComponent()
{
	if (!std::is_base_of<Component::BaseComponent, T>::value) {
		PrintError("Incorrect Type for component");
		return std::weak_ptr<T>();
	}

	std::shared_ptr<T> component = std::make_shared<T>();
	AddComponent(component);
	return component;
}

template<typename T>
inline void Core::GameObject::AddComponent(std::shared_ptr<T> component)
{
	if (!std::is_base_of<Component::BaseComponent, T>::value) {
		PrintError("Incorrect Type for component");
		return;
	}
	component->SetGameObject(weak_from_this());
	m_components.push_back(component);
}

template<typename T>
inline std::vector<Weak<T>> Core::GameObject::GetComponentsInChildren()
{
	std::vector<Weak<T>> list = GetComponents<T>();
	for (Weak<GameObject>& child : m_childs)
	{
		Shared<GameObject> lockedChild = child.lock();
		if (lockedChild)
		{
			std::vector<Weak<T>> newList = lockedChild->GetComponentsInChildren<T>();
			list.insert(list.end(), newList.begin(), newList.end());
		}
	}
	return list;
}


template<typename T>
inline std::vector<Weak<T>> Core::GameObject::GetComponents()
{
	std::vector<Weak<T>> list;
	for (auto& comp : m_components) {
		if (auto castedComp = std::dynamic_pointer_cast<T>(comp)) {
			list.push_back(castedComp);
		}
	}
	return list;
}


template<typename T>
inline std::vector<Shared<T>> Core::GameObject::GetComponentsPrivate()
{
	std::vector<Shared<T>> list;
	for (auto& comp : m_components) {
		if (auto castedComp = std::dynamic_pointer_cast<T>(comp)) {
			list.push_back(castedComp);
		}
	}
	return list;
}

template<typename T>
inline std::vector<Shared<T>> Core::GameObject::GetComponentsInChildrenPrivate()
{
	std::vector<Shared<T>> list = GetComponentsPrivate<T>();
	for (Weak<GameObject>& child : m_childs)
	{
		Shared<GameObject> lockedChild = child.lock();
		if (lockedChild)
		{
			std::vector<Shared<T>> newList = lockedChild->GetComponentsInChildrenPrivate<T>();
			list.insert(list.end(), newList.begin(), newList.end());
		}
	}
	return list;
}


template<typename T>
inline void Core::GameObject::AddComponent(std::shared_ptr<T> component, uint32_t index)
{
	component->SetGameObject(weak_from_this());
	m_components.insert(m_components.begin() + index, component);
}