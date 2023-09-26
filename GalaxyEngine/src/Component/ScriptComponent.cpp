#include "pch.h"
#include "Component/ScriptComponent.h"
namespace GALAXY 
{

	void Component::ScriptComponent::OnCreate()
	{
		m_component->OnCreate();
	}

	void Component::ScriptComponent::OnStart()
	{
		m_component->OnStart();
	}

	void Component::ScriptComponent::OnUpdate()
	{
		m_component->OnUpdate();
	}

	void Component::ScriptComponent::OnDraw()
	{
		m_component->OnDraw();
	}

	void Component::ScriptComponent::OnDestroy()
	{
		m_component->OnDestroy();
	}

	void Component::ScriptComponent::ShowInInspector()
	{
		m_component->ShowInInspector();
	}

}
