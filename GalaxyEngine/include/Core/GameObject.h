#pragma once
#include "GalaxyAPI.h"
#include "Component/Transform.h"
#include <memory>
#include <vector>
#include <string>

namespace GALAXY {
	namespace EditorUI { class Hierarchy; }
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

			void RemoveChild(GameObject* child);
			void RemoveChild(uint32_t index);

			// === Setters === //

			void AddChild(std::weak_ptr<GameObject> child, uint32_t index = -1);

			// Set the parent to the given GameObject
			void SetParent(std::weak_ptr<GameObject> parent);

			// === Getters === //

			Component::Transform* GetTransform() { return m_transform.get(); }

			std::vector<std::weak_ptr<GameObject>> GetChildren();
			std::weak_ptr<GameObject> GetChild(uint32_t index);

			// Return the child index in the list of child
			uint32_t GetChildIndex(GameObject* child);

			std::weak_ptr<GameObject> GetParent();

			bool IsAParent(GameObject* object);

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
		};
	}
}