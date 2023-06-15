#include "pch.h"
#include "Component/IComponent.h"

void GALAXY::Component::BaseComponent::ShowInInspector()
{
	Wrapper::Reflection::ShowInspectorClass(this);
}
