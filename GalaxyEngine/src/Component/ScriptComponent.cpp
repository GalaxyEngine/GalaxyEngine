#include "pch.h"
#include "Component/ScriptComponent.h"

#include "Scripting/ScriptEngine.h"
#include "Scripting/ScriptInstance.h"

namespace GALAXY
{

	std::string Component::ScriptComponent::GetComponentName() const
	{
		if (m_component)
			return m_component->GetComponentName();
		return "Script Component";
	}

	std::shared_ptr<Component::BaseComponent> Component::ScriptComponent::Clone()
	{
		auto copy = std::make_shared<ScriptComponent>(*dynamic_cast<ScriptComponent*>(this));
		copy->m_component = m_component->Clone();
		return copy;
	}

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
		auto variables = GetAllVariables();
		for (auto& variable : variables)
		{
			switch (variable.second)
			{
			case Scripting::VariableType::Unknown:
			{
				ImGui::Text("Unknown type for %s", variable.first.c_str());
			}
			break;
			case Scripting::VariableType::Bool:
			{
				bool* value = GetVariable<bool>(variable.first);
				ImGui::Checkbox(variable.first.c_str(), value);
			}
			break;
			case Scripting::VariableType::Int:
			{
				int* value = GetVariable<int>(variable.first);
				ImGui::InputInt(variable.first.c_str(), value);
			}
			break;
			case Scripting::VariableType::Float:
			{
				float* value = GetVariable<float>(variable.first);
				ImGui::DragFloat(variable.first.c_str(), value);
			}
			break;
			case Scripting::VariableType::Double:
			{
				double* value = GetVariable<double>(variable.first);
				ImGui::InputDouble(variable.first.c_str(), value);
			}
			break;
			case Scripting::VariableType::Vector2:
			{
				Math::Vec2f* value = GetVariable<Math::Vec2f>(variable.first);
				ImGui::DragFloat2(variable.first.c_str(), &value->x);
			}
			break;
			case Scripting::VariableType::Vector3:
			{
				Math::Vec3f* value = GetVariable<Math::Vec3f>(variable.first);
				ImGui::DragFloat3(variable.first.c_str(), &value->x);
			}
			break;
			case Scripting::VariableType::Vector4:
			{
				Math::Vec4f* value = GetVariable<Math::Vec4f>(variable.first);
				ImGui::DragFloat4(variable.first.c_str(), &value->x);
			}
			break;
			default:
				break;
			}
		}
	}

	std::unordered_map<std::string, Scripting::VariableType> Component::ScriptComponent::GetAllVariables() const
	{
		auto scriptInstance = Scripting::ScriptEngine::GetInstance()->GetScriptInstance(m_component->GetComponentName()).lock();
		return scriptInstance->GetAllVariables();
	}

	void Component::ScriptComponent::ReloadScript()
	{
		//auto beforeVariables = GetAllVariables();
		m_component.reset();
		m_component = Scripting::ScriptEngine::GetInstance()->CreateScript(m_scriptName);
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
