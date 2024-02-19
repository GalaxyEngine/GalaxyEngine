#include "pch.h"
#include "Component/ComponentHolder.h"
#include "Component/MeshComponent.h"
#include "Component/DirectionalLight.h"
#include "Component/PointLight.h"
#include "Component/SpotLight.h"

std::vector<std::shared_ptr<Component::BaseComponent>> Component::ComponentHolder::m_componentList;

using namespace Component;
void ComponentHolder::Initialize()
{
	RegisterComponent<MeshComponent>();
	RegisterComponent<DirectionalLight>();
	RegisterComponent<PointLight>();
	RegisterComponent<SpotLight>();
}

void ComponentHolder::UnregisterComponent(const char* componentName)
{
	for (auto it = m_componentList.begin(); it != m_componentList.end(); it++)
	{
		if ((*it)->GetComponentName() == componentName)
		{
			m_componentList.erase(it);
			return;
		}
	}
}

