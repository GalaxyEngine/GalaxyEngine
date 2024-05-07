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

	Resource::Scene* Core::GameObject::GetScene() const
	{
		return m_scene;
	}

	inline Component::Transform* Core::GameObject::GetTransform() const
	{
		return m_transform.get();
	}

	inline Shared<Core::GameObject> Core::GameObject::GetParent() const
	{
		return m_parent.lock();
	}

	inline uint32_t Core::GameObject::GetChildrenCount() const
	{
		return static_cast<uint32_t>(m_children.size());
	}

	void Core::GameObject::SetHierarchyOpen(const bool val)
	{
		m_open = val;
	}

	inline List<Weak<Core::GameObject>> Core::GameObject::GetChildren() const
	{
		List<Weak<GameObject>> weakPtrVector;
		for (const auto& sharedPtr : m_children) {
			weakPtrVector.push_back(sharedPtr);
		}
		return weakPtrVector;
	}

	inline Weak<Core::GameObject> Core::GameObject::GetChild(const uint32_t index)
	{
		if (index < m_children.size())
			return m_children.at(index);
		return {};
	}

	bool Core::GameObject::IsActive() const
	{
		return m_active;
	}

	inline bool Core::GameObject::IsSelected() const
	{
		return m_selected;
	}

	inline void Core::GameObject::SetName(String val)
	{
		m_name = std::move(val);
	}

	template<typename T>
	inline Weak<T> Core::GameObject::AddComponent()
	{
		if (!std::is_base_of_v<Component::BaseComponent, T>) {
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
		if (!std::is_base_of_v<Component::BaseComponent, T>) {
			PrintError("Incorrect Type for component");
			return;
		}
		component->SetGameObject(this);
		m_components.push_back(component);
		component->p_id = static_cast<uint32_t>(m_components.size() - 1);
		component->OnCreate();
	}

	template<typename T>
	inline List<Weak<T>> Core::GameObject::GetComponentsInChildren()
	{
		List<Weak<T>> list = GetComponents<T>();
		for (const Weak<GameObject>& child : m_children)
		{
			if (Shared<GameObject> lockedChild = child.lock())
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

	template <typename T>
	Shared<T> Core::GameObject::GetComponent()
	{
		for (auto& comp : m_components) {
			if (Shared<T> castedComp = std::dynamic_pointer_cast<T>(comp)) {
				return castedComp;
			}
		}
		return {};
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
		return {};
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
		for (const Weak<GameObject>& child : m_children)
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
		component->SetGameObject(this);
		component->p_id = index;
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

	inline bool Core::GameObject::IsAParent(GameObject* object) const
	{
		if (object == this)
			return true;
		if (m_parent.lock())
			return m_parent.lock()->IsAParent(object);
		return false;
	}
}