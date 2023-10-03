#include "pch.h"
#include "Component/ScriptComponent.h"

#include "Scripting/ScriptEngine.h"
#include "Scripting/ScriptInstance.h"

#include <any>
namespace GALAXY
{
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
				if (auto component = *GetVariable<Component::BaseComponent*>(variable.first))
					ImGui::Text("Component %s", component->GetComponentName());
			}
			break;
			case Scripting::VariableType::Bool:
			{
				if (bool* value = GetVariable<bool>(variable.first))
					ImGui::Checkbox(variable.first.c_str(), value);
			}
			break;
			case Scripting::VariableType::Int:
			{
				if (int* value = GetVariable<int>(variable.first))
					ImGui::InputInt(variable.first.c_str(), value);
			}
			break;
			case Scripting::VariableType::Float:
			{
				if (float* value = GetVariable<float>(variable.first))
					ImGui::DragFloat(variable.first.c_str(), value);
			}
			break;
			case Scripting::VariableType::Double:
			{
				if (double* value = GetVariable<double>(variable.first))
					ImGui::InputDouble(variable.first.c_str(), value);
			}
			break;
			case Scripting::VariableType::Vector2:
			{
				if (Math::Vec2f* value = GetVariable<Math::Vec2f>(variable.first))
					ImGui::DragFloat2(variable.first.c_str(), &value->x);
			}
			break;
			case Scripting::VariableType::Vector3:
			{
				if (Math::Vec3f* value = GetVariable<Math::Vec3f>(variable.first))
					ImGui::DragFloat3(variable.first.c_str(), &value->x);
			}
			break;
			case Scripting::VariableType::Vector4:
			{
				if (Math::Vec4f* value = GetVariable<Math::Vec4f>(variable.first))
					ImGui::DragFloat4(variable.first.c_str(), &value->x);
			}
			break;
			case Scripting::VariableType::String:
			{
				if (std::string* value = GetVariable<std::string>(variable.first))
					ImGui::InputText(variable.first.c_str(), value);
			}
			break;
			default:
				break;
			}
		}
	}

	std::any Component::ScriptComponent::GetVariable(const std::string& variableName)
	{
		Scripting::ScriptEngine* scriptEngine = Scripting::ScriptEngine::GetInstance();
		Weak<Scripting::ScriptInstance> scriptInstanceWeakPtr = scriptEngine->GetScriptInstance(GetComponentName());
		Shared<Scripting::ScriptInstance> scriptInstance = scriptInstanceWeakPtr.lock();
		if (scriptInstance)
		{
			Scripting::VariableType variableType = scriptInstance->m_variables.at(variableName);
			switch (variableType)
			{
			case Scripting::VariableType::Unknown:
				break;
			case Scripting::VariableType::Bool:
				return *GetVariable<bool>(variableName);
			case Scripting::VariableType::Int:
				return *GetVariable<int>(variableName);
			case Scripting::VariableType::Float:
				return *GetVariable<float>(variableName);
			case Scripting::VariableType::Double:
				return *GetVariable<double>(variableName);
			case Scripting::VariableType::Vector2:
				return *GetVariable<Vec2f>(variableName);
			case Scripting::VariableType::Vector3:
				return *GetVariable<Vec3f>(variableName);
			case Scripting::VariableType::Vector4:
				return *GetVariable<Vec4f>(variableName);
			case Scripting::VariableType::String:
				return *GetVariable<std::string>(variableName);
				break;
			default:
				break;
			}
		}
		return nullptr;
	}

	void Component::ScriptComponent::SetVariable(const std::string& variableName, std::any value)
	{
		Scripting::ScriptEngine* scriptEngine = Scripting::ScriptEngine::GetInstance();
		Weak<Scripting::ScriptInstance> scriptInstanceWeakPtr = scriptEngine->GetScriptInstance(GetComponentName());
		Shared<Scripting::ScriptInstance> scriptInstance = scriptInstanceWeakPtr.lock();
		if (scriptInstance)
		{
			Scripting::VariableType variableType = scriptInstance->m_variables.at(variableName);
			switch (variableType)
			{
			case Scripting::VariableType::Unknown:
				break;
			case Scripting::VariableType::Bool:
				SetVariable(variableName, std::any_cast<bool>(value));
				break;
			case Scripting::VariableType::Int:
				SetVariable(variableName, std::any_cast<int>(value));
				break;
			case Scripting::VariableType::Float:
				SetVariable(variableName, std::any_cast<float>(value));
				break;
			case Scripting::VariableType::Double:
				SetVariable(variableName, std::any_cast<double>(value));
				break;
			case Scripting::VariableType::Vector2:
				SetVariable(variableName, std::any_cast<Vec2f>(value));
				break;
			case Scripting::VariableType::Vector3:
				SetVariable(variableName, std::any_cast<Vec3f>(value));
				break;
			case Scripting::VariableType::Vector4:
				SetVariable(variableName, std::any_cast<Vec4f>(value));
				break;
			case Scripting::VariableType::String:
				SetVariable(variableName, std::any_cast<Vec4f>(value));
				break;
			default:
				break;
			}
		}
	}

	std::unordered_map<std::string, Scripting::VariableType> Component::ScriptComponent::GetAllVariables() const
	{
		Scripting::ScriptEngine* scriptEngine = Scripting::ScriptEngine::GetInstance();
		auto scriptInstance = scriptEngine->GetScriptInstance(this->GetComponentName()).lock();
		if (scriptInstance)
			return scriptInstance->GetAllVariables();
		return {};
	}

	void Component::ReloadScript::BeforeReloadScript()
	{
		m_scriptName = m_component->GetComponentName();
		auto beforeVariables = m_component->GetAllVariables();
		for (auto& variable : beforeVariables)
		{
			m_tempVariables[variable.first] = m_component->GetVariable(variable.first);
		}
		m_component.reset();
	}

	void Component::ReloadScript::AfterReloadScript()
	{
		m_component = Scripting::ScriptEngine::GetInstance()->CreateScript(m_scriptName);

		ASSERT(m_component);

		auto afterVariables = m_component->GetAllVariables();
		int i = 0;
		for (auto& variable : afterVariables)
		{
			if (m_tempVariables.count(variable.first))
			{
				if (variable.second == Scripting::VariableType::Float)
					std::cout << std::any_cast<float>(m_tempVariables[variable.first]) << std::endl;
				m_component->SetVariable(variable.first, m_tempVariables[variable.first]);
			}
		}
		m_tempVariables.clear();
	}

	void* Component::ScriptComponent::GetVariableVoid(const std::string& variableName)
	{
		return Scripting::ScriptEngine::GetInstance()->GetVariableOfScript(this, GetComponentName(), variableName);
	}

	void Component::ScriptComponent::SetVariableVoid(const std::string& variableName, void* value)
	{
		Scripting::ScriptEngine::GetInstance()->SetVariableOfScript(this, GetComponentName(), variableName, value);
	}
}
