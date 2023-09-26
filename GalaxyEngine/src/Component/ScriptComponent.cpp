#include "pch.h"
#include "Component/ScriptComponent.h"
#include "Scripting/ScriptEngine.h"
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

	void* Component::ScriptComponent::GetVariableVoid(const std::string& variableName)
	{
		return Scripting::ScriptEngine::GetInstance()->GetVariableOfScript(m_component.get(), m_component.get()->GetComponentName(), variableName);
	}

	void Component::ScriptComponent::SetVariableVoid(const std::string& variableName, void* value)
	{
		Scripting::ScriptEngine::GetInstance()->SetVariableOfScript(m_component.get(), m_component.get()->GetComponentName(), variableName, value);
	}

}
