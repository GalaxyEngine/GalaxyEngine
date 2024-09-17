#include "pch.h"
#include "Component/ScriptComponent.h"

#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

void Component::ScriptComponent::InitializeVariablesInfo()
{
	m_variablesInfo = Scripting::ScriptEngine::GetInstance()->GetAllScriptVariablesInfo(GetComponentName());
}

void Component::ScriptComponent::SetupVariables()
{
	InitializeVariablesInfo();

	m_variablesPtr.clear();
	for (const auto& variable : m_variablesInfo) {
		auto value = GetVariable<void*>(variable.first); 
		m_variablesPtr[variable.first] = value;
	}
}

static std::string content;
void Component::ScriptComponent::ShowInInspector()
{
	for (const auto& variable : m_variablesInfo) {
		if (variable.second->displayValue)
		{
			void* variableValue = m_variablesPtr[variable.first];
			if (!variableValue)
				continue;
			variable.second->displayValue(variable.first, variableValue);
		}
	}
}

void Component::ScriptComponent::OnCreate()
{
	// Setup Variables so the ptrs are correct
	SetupVariables();
}

void Component::ScriptComponent::Serialize(CppSer::Serializer& serializer)
{
	for (auto& variable : m_variablesInfo)
	{
		variable.second->Serialize(serializer, variable.first, m_variablesPtr[variable.first]);
	}
}

void Component::ScriptComponent::Deserialize(CppSer::Parser& parser)
{
	for (auto& variable : m_variablesInfo)
	{
		variable.second->Deserialize(parser, variable.first, m_variablesPtr[variable.first]);
	}
}

void Component::ScriptComponent::AfterLoad()
{
	for (auto& variable : m_variablesInfo)
	{
		variable.second->AfterLoad(GetGameObject()->GetScene());
	}
}
