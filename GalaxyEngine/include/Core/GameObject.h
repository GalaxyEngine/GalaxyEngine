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
	namespace Scripting
	{
		class ScriptEngine;
	}
	namespace Utils
	{
		class Serializer;
		class Parser;
	}
	namespace Core {
		class GALAXY_API GameObject : public std::enable_shared_from_this<GameObject>
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
			inline void SetName(std::string val);

			void AddChild(std::weak_ptr<GameObject> child, uint32_t index = -1);

			// Set the parent to the given GameObject
			void SetParent(std::weak_ptr<GameObject> parent);

			template<typename T>
			inline std::weak_ptr<T> AddComponent();

			template<typename T>
			inline void AddComponent(std::shared_ptr<T> component);

			void ChangeComponentIndex(uint32_t prevIndex, uint32_t newIndex);

			// === Getters === //
			inline std::string GetName() const;
			inline uint64_t GetIndex() const;

			Component::Transform* Transform() { return m_transform.get(); }
			std::weak_ptr<GameObject> GetParent();

			std::vector<std::weak_ptr<GameObject>> GetChildren();
			std::vector<std::weak_ptr<GameObject>> GetAllChildren();
			std::weak_ptr<GameObject> GetChild(uint32_t index);

			// Return the child index in the list of child
			uint32_t GetChildIndex(GameObject* child);
			// Return the Component index in the list of components
			uint32_t GetComponentIndex(Component::BaseComponent* component);

			template<typename T>
			inline std::vector<Weak<T>> GetComponentsInChildren();
			template<typename T>
			inline std::vector<Weak<T>> GetComponents();

			Weak<Component::BaseComponent> GetComponentWithIndex(uint32_t index);

			Component::BaseComponent* GetComponentWithName(const std::string& componentName);

			// Check if the object given is a parent of the this
			bool IsAParent(GameObject* object);
			bool IsSibling(const std::vector<std::weak_ptr<GameObject>>& siblings);

			void Serialize(Utils::Serializer& serializer);
			void Deserialize(Utils::Parser& parser);
		private:
			friend class Scene;
			friend Scripting::ScriptEngine;
			uint64_t m_id = -1;
			std::string m_name = "GameObject";
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

		private:
			template<typename T>
			inline std::vector<Shared<T>> GetComponentsPrivate();
			template<typename T>
			inline std::vector<Shared<T>> GetComponentsInChildrenPrivate();

			template<typename T>
			inline void AddComponent(std::shared_ptr<T> component, uint32_t index);
		};
	}
}
#include "Core/GameObject.inl" 
