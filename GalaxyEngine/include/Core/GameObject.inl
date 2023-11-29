#pragma once
#include "Core/GameObject.h"
namespace GALAXY {
	inline std::string Core::GameObject::GetName() const
	{
		return m_name;
	}


	inline Core::UUID Core::GameObject::GetUUID() const
	{
		return m_UUID;
	}
	inline uint64_t Core::GameObject::GetSceneGraphID() const
	{
		return m_sceneGraphID;
	}

	inline List<Weak<Core::GameObject>> Core::GameObject::GetChildren()
	{
		List<Weak<GameObject>> weakPtrVector;
		for (const auto& sharedPtr : m_childs) {
			weakPtrVector.push_back(sharedPtr);
		}
		return weakPtrVector;
	}

	inline Weak<Core::GameObject> Core::GameObject::GetChild(uint32_t index)
	{
		if (index < m_childs.size())
			return m_childs.at(index);
		return Weak<GameObject>();
	}

	inline void Core::GameObject::SetName(String val)
	{
		m_name = std::move(val);
	}

	template<typename T>
	inline Weak<T> Core::GameObject::AddComponent()
	{
		if (!std::is_base_of<Component::BaseComponent, T>::value) {
			PrintError("Incorrect Type for component");
			return std::weak_ptr<T>();
		}

		Shared<T> component = std::make_shared<T>();
		AddComponent(component);
		return component;
	}

	template<typename T>
	inline void Core::GameObject::AddComponent(Shared<T> component)
	{
		if (!std::is_base_of<Component::BaseComponent, T>::value) {
			PrintError("Incorrect Type for component");
			return;
		}
		component->SetGameObject(weak_from_this());
		m_components.push_back(component);
		component->p_id = static_cast<uint32_t>(m_components.size() - 1);
		component->OnCreate();
	}

	template<typename T>
	inline List<Weak<T>> Core::GameObject::GetComponentsInChildren()
	{
		List<Weak<T>> list = GetComponents<T>();
		for (Weak<GameObject>& child : m_childs)
		{
			Shared<GameObject> lockedChild = child.lock();
			if (lockedChild)
			{
				List<Weak<T>> newList = lockedChild->GetComponentsInChildren<T>();
				list.insert(list.end(), newList.begin(), newList.end());
			}
		}
		return list;
	}


	template<typename T>
	inline List<Weak<T>> Core::GameObject::GetComponents()
	{
		List<Weak<T>> list;
		for (auto& comp : m_components) {
			if (auto castedComp = std::dynamic_pointer_cast<T>(comp)) {
				list.push_back(castedComp);
			}
		}
		return list;
	}

	template<typename T>
	inline Weak<T> Core::GameObject::GetWeakComponent()
	{
		for (auto& component : m_components)
		{
			if (auto castedComponent = std::dynamic_pointer_cast<T>(component))
			{
				return castedComponent;
			}
		}
	}



	template<typename T>
	inline List<Shared<T>> Core::GameObject::GetComponentsPrivate()
	{
		List<Shared<T>> list;
		for (auto& comp : m_components) {
			if (auto castedComp = std::dynamic_pointer_cast<T>(comp)) {
				list.push_back(castedComp);
			}
		}
		return list;
	}

	template<typename T>
	inline List<Shared<T>> Core::GameObject::GetComponentsInChildrenPrivate()
	{
		List<Shared<T>> list = GetComponentsPrivate<T>();
		for (Weak<GameObject>& child : m_childs)
		{
			Shared<GameObject> lockedChild = child.lock();
			if (lockedChild)
			{
				List<Shared<T>> newList = lockedChild->GetComponentsInChildrenPrivate<T>();
				list.insert(list.end(), newList.begin(), newList.end());
			}
		}
		return list;
	}


	template<typename T>
	inline void Core::GameObject::AddComponent(Shared<T> component, uint32_t index)
	{
		component->SetGameObject(weak_from_this());
		m_components.insert(m_components.begin() + index, component);
	}


	template<typename T>
	inline Weak<T> Core::GameObject::GetWeakOfComponent(T* component)
	{
		for (auto& componentArray : m_components)
		{
			if (componentArray.get() == component)
			{
				return std::dynamic_pointer_cast<T>(componentArray);
			}
		}
		return {};
	}

	inline bool Core::GameObject::IsAParent(GameObject* object)
	{
		if (object == this)
			return true;
		else if (m_parent.lock())
			return m_parent.lock()->IsAParent(object);
		return false;
	}
}