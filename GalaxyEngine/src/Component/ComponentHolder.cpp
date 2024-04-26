#include "pch.h"
#include "Component/ComponentHolder.h"

#include "Component/CameraComponent.h"
#include "Component/MeshComponent.h"
#include "Component/DirectionalLight.h"
#include "Component/Listener.h"
#include "Component/PointLight.h"
#include "Component/SpotLight.h"
#include "Component/Emitter.h"

std::vector<std::shared_ptr<Component::BaseComponent>> Component::ComponentHolder::m_componentList;

using namespace Component;
void ComponentHolder::Initialize()
{
	RegisterComponent<MeshComponent>();
	RegisterComponent<DirectionalLight>();
	RegisterComponent<PointLight>();
	RegisterComponent<SpotLight>();
	RegisterComponent<CameraComponent>();
	RegisterComponent<Emitter>();
	RegisterComponent<Listener>();
}

void ComponentHolder::UnregisterComponentByName(const std::string& componentName)
{
	for (auto it = m_componentList.begin(); it != m_componentList.end(); it++)
	{
		std::string componentName2 = (*it)->GetComponentName();
		if (componentName2 == componentName)
		{
			m_componentList.erase(it);
			return;
		}
	}
}

void ComponentHolder::Release()
{
	m_componentList.clear();
}

