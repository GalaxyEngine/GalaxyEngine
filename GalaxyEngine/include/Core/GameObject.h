#pragma once
#include "GalaxyAPI.h"
#include "Component/Transform.h"
#include <memory>
#include <vector>
#include <string>

namespace GALAXY {
	namespace EditorUI
	{
		class Hierarchy;
		class Inspector;
	}
	namespace Core {
		class GameObject : public std::enable_shared_from_this<GameObject>
		{
		public:
			GameObject();
			GameObject(const std::string& name);
			GameObject& operator=(const GameObject& other) = default;
			GameObject(const GameObject&) = default;
			GameObject(GameObject&&) noexcept = default;
			virtual ~GameObject() {}

			// Main Methods
			void Initialize();

			void UpdateSelfAndChild();
			void DrawSelfAndChild();

			void RemoveChild(GameObject* child);
			void RemoveChild(uint32_t index);

			void RemoveComponent(Component::BaseComponent* component);

			// === Setters === //

			void AddChild(std::weak_ptr<GameObject> child, uint32_t index = -1);

			// Set the parent to the given GameObject
			void SetParent(std::weak_ptr<GameObject> parent);

			template<typename T>
			inline std::weak_ptr<T> AddComponent();

			template<typename T>
			inline void AddComponent(std::shared_ptr<T> component);

			void ChangeComponentIndex(uint32_t prevIndex, uint32_t newIndex);

			// === Getters === //

			Component::Transform* Transform() { return m_transform.get(); }
			std::weak_ptr<GameObject> GetParent();

			std::vector<std::weak_ptr<GameObject>> GetChildren();
			std::weak_ptr<GameObject> GetChild(uint32_t index);

			// Return the child index in the list of child
			uint32_t GetChildIndex(GameObject* child);
			// Return the Component index in the list of components
			uint32_t GetComponentIndex(Component::BaseComponent* component);

			template<typename T>
			std::vector<Weak<T>> GetComponentsInChildren()
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
			std::vector<Weak<T>> GetComponents()
			{
				std::vector<Weak<T>> list;
				for (auto& comp : m_components) {
					if (auto castedComp = std::dynamic_pointer_cast<T>(comp)) {
						list.push_back(castedComp);
					}
				}
				return list;
			}


			// Check if the object givent is a parent of the this
			bool IsAParent(GameObject* object);
			bool IsSibling(const std::vector<std::weak_ptr<GameObject>>& siblings);

		private:
			friend class Scene;
			uint64_t m_id = -1;
			std::string m_name = "GameObject";;
			std::weak_ptr<GameObject> m_parent;
			std::vector<std::weak_ptr<GameObject>> m_childs;
			std::vector<std::shared_ptr<Component::BaseComponent>> m_components;

			std::unique_ptr<Component::Transform> m_transform;

			// Hierarchy Parameters
			friend EditorUI::Hierarchy;

			bool m_open = false;
			bool m_selected = false;

			friend EditorUI::Inspector;
			bool m_active = true;
		};
	}
}
#include "Core/GameObject.inl" 
