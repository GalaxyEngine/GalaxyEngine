#include "pch.h"
#include "Component/IComponent.h"
#include "Core/GameObject.h"

namespace GALAXY {
	void Component::BaseComponent::RemoveFromGameObject()
	{
		if (const Shared<Core::GameObject> gameObject = GetGameObject())
		{
			gameObject->RemoveComponent(this);
		}
	}

	bool Component::BaseComponent::IsEnable() const
	{
		return p_enable && p_gameObject.lock()->IsActive();
	}

	Component::Transform* Component::BaseComponent::GetTransform() const
	{
		return p_gameObject.lock()->GetTransform();
	}

	Component::BaseComponent::BaseComponent()
	{

	}

}