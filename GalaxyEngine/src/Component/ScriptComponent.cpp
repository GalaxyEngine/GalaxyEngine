#include "pch.h"
#include "Component/ScriptComponent.h"

#include "Scripting/ScriptEngine.h"
#include "Scripting/ScriptInstance.h"

#include "Core/SceneHolder.h"
#include "Core/Scene.h"

#include <any>
namespace GALAXY
{
	std::vector<const char*> Component::ScriptComponent::GetComponentNames() const
	{
		auto vector = BaseComponent::GetComponentNames();
		vector.insert(vector.end(), ScriptComponent::GetComponentName());
		return vector;
	}

	void Component::ScriptComponent::ShowInInspector()
	{
		auto variables = GetAllVariables();
		for (auto& variable : variables)
		{
			switch (variable.second.type)
			{
			case Scripting::VariableType::Unknown:
			{
				ImGui::Text("Unknown type for %s", variable.first.c_str());
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
			case Scripting::VariableType::Component:
			{
				if (Component::BaseComponent** value = GetVariable<Component::BaseComponent*>(variable.first))
				{
					Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x / 2.f, 0);
					ImGui::Button(*value ? (*value)->GetComponentName() : "None", buttonSize);
					if (*value) {
						ImGui::SameLine();
						if (ImGui::Button("Reset", Vec2f(ImGui::GetContentRegionAvail().x, 0)))
						{
							(*value) = nullptr;
						}
					}

					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS")) {
							// Check if the payload data type matches
							std::vector<uint64_t> indices;
							if (payload->DataSize % sizeof(uint64_t) == 0)
							{
								uint64_t* payloadData = static_cast<uint64_t*>(payload->Data);
								uint64_t payloadSize = payload->DataSize / sizeof(uint64_t);
								indices.assign(payloadData, payloadData + payloadSize);
							}
							for (size_t i = 0; i < indices.size(); i++) {
								// Get the gameobject with the indices
								std::weak_ptr<Core::GameObject> payloadGameObject = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetWithIndex(indices[i]);
								if (auto component = payloadGameObject.lock()->GetComponentWithName(variable.second.typeName))
								{
									// Get Component of the good type with the name
									(*value) = component;
									return;
								}
							}
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::SameLine();
					ImGui::Text("(%s)", variable.second.typeName.c_str());
				}
			}
			break;
			case Scripting::VariableType::GameObject:
			{
				if (Core::GameObject** value = GetVariable<Core::GameObject*>(variable.first))
				{
					Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x / 2.f, 0);
					ImGui::Button(*value ? (*value)->GetName().c_str() : "None", buttonSize);
					if (*value) {
						ImGui::SameLine();
						if (ImGui::Button("Reset", Vec2f(ImGui::GetContentRegionAvail().x, 0)))
							(*value) = nullptr;
					}

					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS")) {
							// Check if the payload data type matches
							std::vector<uint64_t> indices;
							if (payload->DataSize % sizeof(uint64_t) == 0)
							{
								uint64_t* payloadData = static_cast<uint64_t*>(payload->Data);
								uint64_t payloadSize = payload->DataSize / sizeof(uint64_t);
								indices.assign(payloadData, payloadData + payloadSize);
							}
							// Get the gameobject with the first index
							*value = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetWithIndex(indices[0]).lock().get();
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::SameLine();
					ImGui::TextUnformatted("(GameObject)");
				}
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
			Scripting::VariableType variableType = scriptInstance->m_variables.at(variableName).type;
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
			case Scripting::VariableType::Component:
				return *GetVariable<Component::BaseComponent*>(variableName);
			case Scripting::VariableType::GameObject:
				return *GetVariable<Core::GameObject*>(variableName);
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
			Scripting::VariableType variableType = scriptInstance->m_variables.at(variableName).type;
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
				SetVariable(variableName, std::any_cast<std::string>(value));
				break;
			case Scripting::VariableType::Component:
				SetVariable(variableName, std::any_cast<Component::BaseComponent*>(value));
				break;
			case Scripting::VariableType::GameObject:
				SetVariable(variableName, std::any_cast<Core::GameObject*>(value));
				break;
			default:
				break;
			}
		}
	}

	std::unordered_map<std::string, Scripting::VariableData> Component::ScriptComponent::GetAllVariables() const
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

		// Create a tempvariable map with the name of the variable and a pair with the value and the type
		for (auto& variable : beforeVariables)
		{
			auto variableValue = m_component->GetVariable(variable.first);
			if (variable.second.type == Scripting::VariableType::Component)
			{
				auto component = std::any_cast<Component::BaseComponent*>(variableValue);
				std::tuple<uint64_t, uint32_t, std::string> tuple;
				if (component) {
					auto gameObjectID = component->gameObject.lock()->GetIndex();
					auto componentID = component->gameObject.lock()->GetComponentIndex(component);
					tuple = std::make_tuple(gameObjectID, componentID, std::string(component->GetComponentName()));
				}
				variableValue = tuple;
			}
			else if (variable.second.type == Scripting::VariableType::GameObject)
			{
				uint64_t id = -1;
				if (auto gameObject = std::any_cast<Core::GameObject*>(variableValue)) 
				{
					id = gameObject->GetIndex();
				}
				variableValue = id;
			}
			m_tempVariables[variable.first] = std::make_pair(variableValue, variable.second);
		}
		m_component.reset();
	}

	void Component::ReloadScript::AfterReloadScript()
	{
		m_component = Scripting::ScriptEngine::GetInstance()->CreateScript(GetScriptName());

		ASSERT(m_component);

		auto afterVariables = m_component->GetAllVariables();
		int i = 0;
		for (auto& variable : afterVariables)
		{
			// Check if the variable is still there and if the variable has the same type
			if (m_tempVariables.count(variable.first) && m_tempVariables[variable.first].second.type == variable.second.type)
			{
				auto variableValue = m_tempVariables[variable.first];
				// Set with index of Component + Gameobject
				if (variable.second.type == Scripting::VariableType::Component)
				{
					std::tuple<uint64_t, uint32_t, std::string> tuple = std::any_cast<std::tuple<uint64_t, uint32_t, std::string>>(variableValue.first);
					auto gameObject = Core::SceneHolder::GetCurrentScene()->GetWithIndex(std::get<0>(tuple));
					if (!gameObject.lock())
						continue;
					// Check if component exist and the same type of the previous one
					uint32_t componentID = std::get<1>(tuple);
					std::string componentName = std::get<2>(tuple);
					auto component = gameObject.lock()->GetComponentWithIndex(componentID).lock();
					if (component && component->GetComponentName() == componentName)
						variableValue.first = component.get();
					else {
						m_missingComponentRefs[variable.first] = tuple;
						continue;
					}
				}
				// Set with index of GameObject
				else if (variable.second.type == Scripting::VariableType::GameObject)
				{
					uint64_t index = std::any_cast<uint64_t>(variableValue.first);
					if (auto gameObject = Core::SceneHolder::GetCurrentScene()->GetWithIndex(index).lock())
						variableValue.first = gameObject.get();
				}
				m_component->SetVariable(variable.first, variableValue);
			}
		}
		m_tempVariables.clear();
	}

	void Component::ReloadScript::SyncComponentsValues()
	{
		if (m_missingComponentRefs.size() == 0)
			return;
		for (auto& refs : m_missingComponentRefs)
		{
			auto gameObject = Core::SceneHolder::GetCurrentScene()->GetWithIndex(std::get<0>(refs.second));
			if (!gameObject.lock())
				continue;
			// Check if component exist and the same type of the previous one
			uint32_t componentID = std::get<1>(refs.second);
			std::string componentName = std::get<2>(refs.second);
			auto component = gameObject.lock()->GetComponentWithIndex(componentID).lock();
			if (component && component->GetComponentName() == componentName)
				m_component->SetVariable(refs.first, component.get());
		}
	}

	void* Component::ScriptComponent::GetVariableVoid(const std::string& variableName)
	{
		if (!this)
			return nullptr;
		void* variableVoid = Scripting::ScriptEngine::GetInstance()->GetVariableOfScript(this, GetComponentName(), variableName);
		return variableVoid;
	}

	void Component::ScriptComponent::SetVariableVoid(const std::string& variableName, void* value)
	{
		Scripting::ScriptEngine::GetInstance()->SetVariableOfScript(this, GetComponentName(), variableName, value);
	}
}
