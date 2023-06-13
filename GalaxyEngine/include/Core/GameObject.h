#pragma once
#include "GalaxyAPI.h"
#include "Component/Transform.h"
#include <memory>
namespace GALAXY::Core {
	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		GameObject(const std::string& name) : m_name(name) {}
		GameObject() {}
		GameObject& operator=(const GameObject& other) = default;
		GameObject(const GameObject&) = default;
		GameObject(GameObject&&) noexcept = default;
		virtual ~GameObject() {}

		Component::Transform* GetTransform() { return m_transform.get(); }

	private:
		std::string m_name;
		std::weak_ptr<GameObject> m_parent;
		std::vector<std::shared_ptr<GameObject>> m_childs;

		std::unique_ptr<Component::Transform> m_transform;
	};

}

