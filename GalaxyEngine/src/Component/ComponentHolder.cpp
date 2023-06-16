#include "pch.h"
#include "Component/ComponentHolder.h"
#include "Component/MeshComponent.h"

std::vector<std::shared_ptr<Component::BaseComponent>> Component::ComponentHolder::m_componentList;

using namespace Component;
void ComponentHolder::Initialize()
{
	RegisterComponent<MeshComponent>();
}


