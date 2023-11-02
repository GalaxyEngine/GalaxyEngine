#include "pch.h"
#include "Component/IComponent.h"
#include "Core/GameObject.h"

namespace GALAXY {
	void Component::BaseComponent::RemoveFromGameObject()
	{
		if (Shared<Core::GameObject> gameObject = GetGameObject())
		{
			gameObject->RemoveComponent(this);
		}
	}

	Component::BaseComponent::BaseComponent()
	{

	}

}