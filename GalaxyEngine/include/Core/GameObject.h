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
			GameObject(const std::string& name) : m_name(name) {}
			GameObject() {}
			GameObject& operator=(const GameObject& other) = default;
			GameObject(const GameObject&) = default;
			GameObject(GameObject&&) noexcept = default;
			virtual ~GameObject() {}

			void RemoveChild(const std::shared_ptr<GameObject>& child);
			void RemoveChild(uint32_t index);

			// === Setters === //

			void AddChild(std::shared_ptr<GameObject> child);
			void AddChild(GameObject* child);

			void SetParent(GameObject* parent);
			void SetParent(std::shared_ptr<GameObject> parent);

			// === Getters === //

			Component::Transform* GetTransform() { return m_transform.get(); }

			std::vector<std::weak_ptr<GameObject>> GetChildren();
			std::weak_ptr<GameObject> GetChild(uint32_t index);

			std::weak_ptr<GameObject> GetParent();

		private:
			std::string m_name;
			std::weak_ptr<GameObject> m_parent;
			std::vector<std::shared_ptr<GameObject>> m_childs;

			std::unique_ptr<Component::Transform> m_transform;

			// Hierarchy Parameters
			friend EditorUI::Hierarchy;

			bool m_open = false;
			bool m_selected = false;
		};
	}
}