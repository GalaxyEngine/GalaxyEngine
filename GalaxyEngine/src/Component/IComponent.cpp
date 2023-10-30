#include "pch.h"
#include "Component/IComponent.h"
#include "Core/GameObject.h"

namespace GALAXY {
	void Component::BaseComponent::RemoveFromGameObject()
	{
		if (gameObject.lock())
		{
			gameObject.lock()->RemoveComponent(this);
		}
	}

	Component::BaseComponent::BaseComponent()
	{

	}

}