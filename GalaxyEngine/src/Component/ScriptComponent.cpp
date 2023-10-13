#include "pch.h"
#include "Component/ScriptComponent.h"

#include "Scripting/ScriptEngine.h"
#include "Scripting/ScriptInstance.h"

#include "Core/SceneHolder.h"
#include "Core/Scene.h"

#include "Utils/Parser.h"

#define GETVARIABLE(x)\
	if (!variable.isAList)\
		return *GetVariable<x>(variableName);\
	else\
		return *GetVariable<std::vector<x>>(variableName)

#define SETVARIABLE(x)\
	if (!variable.isAList)\
		SetVariable(variableName, std::any_cast<x>(value));\
	else\
		SetVariable(variableName, std::any_cast<std::vector<x>>(value))\

#define GET_VARIABLE(x)\
		(*GetVariable<x>(variable.first));

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
			DisplayVariableField(variable);
		}
	}


	std::any Component::ScriptComponent::GetVariable(const std::string& variableName)
	{
		Scripting::ScriptEngine* scriptEngine = Scripting::ScriptEngine::GetInstance();
		Weak<Scripting::ScriptInstance> scriptInstanceWeakPtr = scriptEngine->GetScriptInstance(GetComponentName());
		Shared<Scripting::ScriptInstance> scriptInstance = scriptInstanceWeakPtr.lock();
		if (scriptInstance)
		{
			Scripting::VariableData variable = scriptInstance->m_variables.at(variableName);
			Scripting::VariableType variableType = variable.type;
			switch (variableType)
			{
			case Scripting::VariableType::Unknown:
				break;
			case Scripting::VariableType::Bool:
				GETVARIABLE(bool);
			case Scripting::VariableType::Int:
				GETVARIABLE(int);
			case Scripting::VariableType::Float:
				GETVARIABLE(float);
			case Scripting::VariableType::Double:
				GETVARIABLE(double);
			case Scripting::VariableType::Vector2:
				GETVARIABLE(Vec2f);
			case Scripting::VariableType::Vector3:
				GETVARIABLE(Vec3f);
			case Scripting::VariableType::Vector4:
				GETVARIABLE(Vec4f);
			case Scripting::VariableType::String:
				GETVARIABLE(std::string);
			case Scripting::VariableType::Component:
				GETVARIABLE(Component::BaseComponent*);
			case Scripting::VariableType::GameObject:
				GETVARIABLE(Core::GameObject*);
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
			Scripting::VariableData variable = scriptInstance->m_variables.at(variableName);
			Scripting::VariableType variableType = variable.type;
			switch (variableType)
			{
			case Scripting::VariableType::Unknown:
				break;
			case Scripting::VariableType::Bool:
				SETVARIABLE(bool);
				break;
			case Scripting::VariableType::Int:
				SETVARIABLE(int);
				break;
			case Scripting::VariableType::Float:
				SETVARIABLE(float);
				break;
			case Scripting::VariableType::Double:
				SETVARIABLE(double);
				break;
			case Scripting::VariableType::Vector2:
				SETVARIABLE(Vec2f);
				break;
			case Scripting::VariableType::Vector3:
				SETVARIABLE(Vec3f);
				break;
			case Scripting::VariableType::Vector4:
				SETVARIABLE(Vec4f);
				break;
			case Scripting::VariableType::String:
				SETVARIABLE(std::string);
				break;
			case Scripting::VariableType::Component:
				SETVARIABLE(Component::BaseComponent*);
				break;
			case Scripting::VariableType::GameObject:
				SETVARIABLE(Core::GameObject*);
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

	void Component::ScriptComponent::Serialize(Utils::Serializer& serializer)
	{
		for (auto& variable : GetAllVariables())
		{
			if (variable.second.isAList)
			{
				//TODO:
				//serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << std::any_cast<std::vector<Scripting::VariableData>>(variable.second);
			}
			else
			{
				switch (variable.second.type)
				{
				case Scripting::VariableType::Unknown:
					break;
				case Scripting::VariableType::Bool:
					serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << GET_VARIABLE(bool);
					break;
				case Scripting::VariableType::Int:
					serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << GET_VARIABLE(int);
					break;
				case Scripting::VariableType::Float:
					serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << GET_VARIABLE(float);
					break;
				case Scripting::VariableType::Double:
					serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << GET_VARIABLE(double);
					break;
				case Scripting::VariableType::Vector2:
					serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << GET_VARIABLE(Vec2f);
					break;
				case Scripting::VariableType::Vector3:
					serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << GET_VARIABLE(Vec3f);
					break;
				case Scripting::VariableType::Vector4:
					serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << GET_VARIABLE(Vec4f);
					break;
				case Scripting::VariableType::String:
					serializer << Utils::PAIR::KEY << variable.first << Utils::PAIR::VALUE << GET_VARIABLE(std::string);
					break;
				case Scripting::VariableType::Component:
					//TODO:
					break;
				case Scripting::VariableType::GameObject:
					//TODO:
					break;
				default:
					break;
				}
			}
		}
	}

	void Component::ScriptComponent::Deserialize(Utils::Parser& parser)
	{

	}

#pragma region ReloadScript
	void Component::ReloadScript::BeforeReloadScript()
	{
		m_scriptName = m_component->GetComponentName();
		auto beforeVariables = m_component->GetAllVariables();

		// Create a tempvariable map with the name of the variable and a pair with the value and the type
		for (const auto& variable : beforeVariables)
		{
			auto variableValue = m_component->GetVariable(variable.first);
			if (variable.second.type == Scripting::VariableType::Component)
			{
				if (!variable.second.isAList) {
					variableValue = ConvertComponentToInfo(variableValue);
				}
				else {
					// If it's a list
					std::vector<Component::BaseComponent*> vector = std::any_cast<std::vector<Component::BaseComponent*>>(variableValue);
					std::vector<ComponentInfo> vectorValue(vector.size());
					for (int i = 0; i < vectorValue.size(); i++)
					{
						auto anyValue = ConvertComponentToInfo(vector[i]);
						vectorValue[i] = std::any_cast<ComponentInfo>(anyValue);
					}
					variableValue = vectorValue;
				}
			}
			else if (variable.second.type == Scripting::VariableType::GameObject)
			{
				if (!variable.second.isAList) {
					variableValue = ConvertGameObjectToID(variableValue);
				}
				else {
					// If it's a list
					std::vector<Core::GameObject*> vector = std::any_cast<std::vector<Core::GameObject*>>(variableValue);
					std::vector<uint64_t> vectorValue(vector.size());
					for (int i = 0; i < vectorValue.size(); i++)
					{
						auto anyValue = ConvertGameObjectToID(vector[i]);
						vectorValue[i] = std::any_cast<uint64_t>(anyValue);
					}
					variableValue = vectorValue;
				}
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
					if (variableValue.second.isAList && variable.second.isAList)
					{
						// If it's a list
						auto listOfInfo = std::any_cast<std::vector<ComponentInfo>>(variableValue.first);
						std::vector<Component::BaseComponent*> listOfComponent(listOfInfo.size());
						// Go trough all componentInfo
						for (int i = 0; i < listOfComponent.size(); i++)
						{
							Component::BaseComponent* component;
							std::any value = listOfInfo[i];
							if (!ConvertInfoToComponent(value)) {
								ComponentInfo info = std::any_cast<ComponentInfo>(listOfInfo[i]);
								info.indexOnList = i;
								info.variableName = variable.first;
								m_missingComponentRefs[variable.first + std::to_string(i)] = info;
							}
							else {
								component = std::any_cast<Component::BaseComponent*>(value);
								listOfComponent[i] = component;
							}
						}
						variableValue.first = listOfComponent;
					}
					else if (variable.second.isAList || variableValue.second.isAList) 
					{
						// If was a list and not now or it's a list now
						return;
					}
					else if (!ConvertInfoToComponent(variableValue.first)) {
						ComponentInfo tuple = std::any_cast<ComponentInfo>(variableValue.first);
						m_missingComponentRefs[variable.first] = tuple;
					}
				}
				// Set with index of GameObject
				else if (variable.second.type == Scripting::VariableType::GameObject)
				{
					if (variableValue.second.isAList && variable.second.isAList)
					{
						// If it's a list
						auto listOfID = std::any_cast<std::vector<uint64_t>>(variableValue.first);
						std::vector<Core::GameObject*> listOfGameObject(listOfID.size());
						// Go trough all gameObjectIDs
						for (int i = 0; i < listOfGameObject.size(); i++)
						{
							std::any id = listOfID[i];
							auto anyGameObject = ConvertIDToGameObject(id);
							listOfGameObject[i] = std::any_cast<Core::GameObject*>(anyGameObject);
						}
						variableValue.first = listOfGameObject;
					}
					else if (variable.second.isAList || variableValue.second.isAList) 
					{
						// If was a list and not now or it's a list now
						return;
					}
					else
						variableValue.first = ConvertIDToGameObject(variableValue.first);
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
			auto gameObject = Core::SceneHolder::GetCurrentScene()->GetWithIndex(refs.second.componentID);
			if (!gameObject.lock())
				continue;
			// Check if component exist and the same type of the previous one
			uint32_t componentID = refs.second.componentID;
			std::string componentName = refs.second.componentName;
			auto component = gameObject.lock()->GetComponentWithIndex(componentID).lock();

			if (component && component->GetComponentName() == componentName) 
			{
				if (!refs.second.indexOnList.has_value()) {
					m_component->SetVariable(refs.first, component.get());
				}
				else {
					// The Component is inside a list
					std::vector<Component::BaseComponent*>* vector = m_component->GetVariable<std::vector<Component::BaseComponent*>>(refs.second.variableName);
					(*vector)[refs.second.indexOnList.value()] = component.get();
					m_component->SetVariable(refs.second.variableName, *vector);
				}
			}
		}
	}

	std::any Component::ReloadScript::ConvertComponentToInfo(const std::any& value)
	{
		auto component = std::any_cast<Component::BaseComponent*>(value);
		ComponentInfo info;
		if (component) {
			auto gameObjectID = component->gameObject.lock()->GetIndex();
			auto componentID = component->gameObject.lock()->GetComponentIndex(component);
			info = ComponentInfo(gameObjectID, componentID, std::string(component->GetComponentName()));
		}
		return info;
	}

	std::any Component::ReloadScript::ConvertGameObjectToID(const std::any& value)
	{
		uint64_t id = -1;
		if (auto gameObject = std::any_cast<Core::GameObject*>(value))
		{
			id = gameObject->GetIndex();
		}
		return id;
	}

	bool Component::ReloadScript::ConvertInfoToComponent(std::any& value)
	{
		ComponentInfo info = std::any_cast<ComponentInfo>(value);
		auto gameObject = Core::SceneHolder::GetCurrentScene()->GetWithIndex(info.gameObjectID);
		if (!gameObject.lock())
			return true;
		// Check if component exist and the same type of the previous one
		uint32_t componentID = info.componentID;
		std::string componentName = info.componentName;
		auto component = gameObject.lock()->GetComponentWithIndex(componentID).lock();
		if (component && component->GetComponentName() == componentName) {
			value = component.get();
			return true;
		}
		else {
			value = nullptr;
			return false;
		}
	}

	std::any Component::ReloadScript::ConvertIDToGameObject(const std::any& value)
	{
		uint64_t index = std::any_cast<uint64_t>(value);
		if (auto gameObject = Core::SceneHolder::GetCurrentScene()->GetWithIndex(index).lock())
			return gameObject.get();
		return nullptr;
	}

#pragma endregion

#pragma region DisplayVariableField
	template<typename T> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, T* value)
	{
		
	}

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, bool* value)
	{
		if (value)
			ImGui::Checkbox(variable.first.c_str(), value);
	}

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, int* value)
	{
		if (value)
			ImGui::InputInt(variable.first.c_str(), value);
	}

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, float* value)
	{
		if (value)
			ImGui::DragFloat(variable.first.c_str(), value);
	}

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, double* value)
	{
		if (value)
			ImGui::InputDouble(variable.first.c_str(), value);
	}

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, Vec2f* value)
	{
		if (value)
			ImGui::DragFloat2(variable.first.c_str(), &value->x);
	}

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, Vec3f* value)
	{
		if (value)
			ImGui::DragFloat3(variable.first.c_str(), &value->x);
	}

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, Vec4f* value)
	{
		if (value)
			ImGui::ColorPicker4(variable.first.c_str(), &value->x);
	}

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, std::string* value)
	{
		//if (value)
			Wrapper::GUI::InputText(variable.first.c_str(), value);
	}

	template<> void Component::ScriptComponent::DisplayVariableT<Component::BaseComponent*>(const std::pair<std::string, Scripting::VariableData>& variable, Component::BaseComponent** value)
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

	template<> void Component::ScriptComponent::DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, Core::GameObject** value)
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

	void Component::ScriptComponent::DisplayVariableField(const std::pair<std::string, Scripting::VariableData>& variable)
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
			DisplayAndManageVariable<bool>(variable);
		}
		break;
		case Scripting::VariableType::Int:
		{
			DisplayAndManageVariable<int>(variable);
		}
		break;
		case Scripting::VariableType::Float:
		{
			DisplayAndManageVariable<float>(variable);
		}
		break;
		case Scripting::VariableType::Double:
		{
			DisplayAndManageVariable<double>(variable);
		}
		break;
		case Scripting::VariableType::Vector2:
		{
			DisplayAndManageVariable<Vec2f>(variable);
		}
		break;
		case Scripting::VariableType::Vector3:
		{
			DisplayAndManageVariable<Vec3f>(variable);
		}
		break;
		case Scripting::VariableType::Vector4:
		{
			DisplayAndManageVariable<Vec4f>(variable);
		}
		break;
		case Scripting::VariableType::String:
		{
			DisplayAndManageVariable<std::string>(variable);
		}
		break;
		case Scripting::VariableType::Component:
		{
			DisplayAndManageVariable<Component::BaseComponent*>(variable);
		}
		break;
		case Scripting::VariableType::GameObject:
		{
			DisplayAndManageVariable<Core::GameObject*>(variable);
		}
		break;
		default:
			break;
		}
	}
#pragma endregion

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
