#include "pch.h"
#include "Component/IComponent.h"
#include "Core/GameObject.h"

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

std::vector<const char*> Component::BaseComponent::GetComponentNames() const
{
	return { BaseComponent::GetComponentName() };
}
