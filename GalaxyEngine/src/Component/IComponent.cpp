#include "pch.h"
#include "Component/IComponent.h"
#include "Core/GameObject.h"

namespace GALAXY {
	void Component::BaseComponent::RemoveFromGameObject()
	{
		if (Core::GameObject* gameObject = GetGameObject())
		{
			gameObject->RemoveComponent(this);
		}
	}

	bool Component::BaseComponent::IsEnable() const
	{
		return p_enable && p_gameObject->IsActive();
	}

	Component::Transform* Component::BaseComponent::GetTransform() const
	{
		return p_gameObject->GetTransform();
	}

	Component::BaseComponent::BaseComponent()
	{

	}

}