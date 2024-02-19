#include "pch.h"
#include "Scripting/ScriptEngine.h"

#include "ScriptEngine.h"


#include "Core/SceneHolder.h"
#include "Core/GameObject.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#include "Component/ComponentHolder.h"
#include "Component/ScriptComponent.h"

namespace GALAXY
{
	using namespace Scripting;
	std::unordered_map<std::string, VariableType> variableTypeMap =
	{
		{"bool", VariableType::Bool},
		{"int", VariableType::Int},
		{"float", VariableType::Float},
		{"double", VariableType::Double},
		{"string", VariableType::String},
		{"Vec2f", VariableType::Vector2f},
		{"Vec3f", VariableType::Vector3f},
		{"Vec4f", VariableType::Vector4f},
		{"Quat", VariableType::Quaternion},
	};

	Scripting::VariableInfo::VariableInfo(const GS::Property& variable)
	{
		this->args = variable.args;
		this->name = variable.name;
		this->typeName = variable.type;

		std::string variableType = variable.type;
		if (variableType.find("::") != std::string::npos)
		{
			variableType = variable.type.substr(variable.type.find_last_of("::") + 1);
			if (variableType.find("vector<") != std::string::npos)
			{
				this->isAList = true;
				variableType = variableType.substr(variableType.find_last_of("<") + 1);
				variableType = variableType.substr(0, variableType.find_last_of(">"));
			}
		}

		if (variableTypeMap.contains(variableType))
			this->type = variableTypeMap[variableType];
		else
			this->type = VariableType::Unknown;

		SetDisplayValue();
	}

	void Scripting::VariableInfo::SetDisplayValue()
	{
		switch (this->type)
		{
		case VariableType::Bool:
			ManageValue<bool>();
			break;
		case VariableType::Int:
			ManageValue<int>();
			break;
		case VariableType::Float:
			ManageValue<float>();
			break;
		case VariableType::Double:
			ManageValue<double>();
			break;
		case VariableType::String:
			ManageValue<std::string>();
			break;
		case VariableType::Vector2f:
			ManageValue<Vec2f>();
			break;
		case VariableType::Vector3f:
			ManageValue<Vec3f>();
			break;
		case VariableType::Vector4f:
			ManageValue<Vec4f>();
			break;
		case VariableType::Quaternion:
			ManageValue<Quat>();
			break;
		default:
			break;
		}
	}

	template<typename T>
	void Scripting::VariableInfo::DisplayValue(const std::string& name, void* value)
	{
		ImGui::Text(name.c_str());
	}

	template<>
	void Scripting::VariableInfo::DisplayValue<int>(const std::string& name, void* value)
	{
		ImGui::DragInt(name.c_str(), (int*)value);
	}

	template<>
	void Scripting::VariableInfo::DisplayValue<float>(const std::string& name, void* value)
	{
		ImGui::DragFloat(name.c_str(), (float*)value, 0.01f);
	}

	template<>
	void Scripting::VariableInfo::DisplayValue<double>(const std::string& name, void* value)
	{
		ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, value);
	}

	template<>
	void Scripting::VariableInfo::DisplayValue<std::string>(const std::string& name, void* value)
	{
		Wrapper::GUI::InputText(name.c_str(), (std::string*)value);
	}

	template<>
	void Scripting::VariableInfo::DisplayValue<Vec2f>(const std::string& name, void* value)
	{
		Vec3f v = *(Vec3f*)value;
		ImGui::DragFloat3(name.c_str(), &v.x, 0.01f);
	}

	template<>
	void Scripting::VariableInfo::DisplayValue<Vec3f>(const std::string& name, void* value)
	{
		Vec3f v = *(Vec3f*)value;
		ImGui::DragFloat3(name.c_str(), &v.x, 0.01f);
	}

	template<>
	void Scripting::VariableInfo::DisplayValue<Vec4f>(const std::string& name, void* value)
	{
		Vec4f v = *(Vec4f*)value;
		ImGui::ColorEdit4(name.c_str(), &v.x, 0.01f);
	}

	template<>
	void Scripting::VariableInfo::DisplayValue<Quat>(const std::string& name, void* value)
	{
		Vec4f v = *(Vec4f*)value;
		ImGui::DragFloat4(name.c_str(), &v.x, 0.01f);
	}



	std::unique_ptr<Scripting::ScriptEngine> Scripting::ScriptEngine::s_instance;

	Scripting::ScriptEngine::ScriptEngine()
	{
		auto projectPath = Resource::ResourceManager::GetInstance()->GetProjectPath();
		m_scriptEngine = GS::ScriptEngine::Get();
		m_scriptEngine->SetCopyToFolder(std::filesystem::current_path() / "ProjectsDLL");
		m_scriptEngine->SetHeaderGenFolder(projectPath / "Generate" / "Headers");
	}

	void Scripting::ScriptEngine::RegisterScriptComponents()
	{
		for (auto& instance : m_scriptEngine->GetAllScriptInstances())
		{
			auto scriptComp = static_cast<Component::ScriptComponent*>(instance.second->m_constructor());
			scriptComp->SetupVariables();
			Component::ComponentHolder::RegisterComponent(scriptComp);
		}
	}

	void Scripting::ScriptEngine::UnregisterScriptComponents()
	{
		for (auto& instance : m_scriptEngine->GetAllScriptInstances())
		{
			Component::ComponentHolder::UnregisterComponent(instance.first.c_str());
		}
	}

	Scripting::ScriptEngine* Scripting::ScriptEngine::GetInstance()
	{
		if (s_instance == nullptr)
		{
			s_instance = std::make_unique<ScriptEngine>();
		}
		return s_instance.get();
	}

	void Scripting::ScriptEngine::LoadDLL(const std::filesystem::path& dllPath)
	{
		if (!m_scriptEngine->LoadDLL(dllPath))
		{
			PrintError("Failed to load DLL: %s", dllPath.string().c_str());
			return;
		}
		m_dllPath = dllPath;
	}


	void Scripting::ScriptEngine::ReloadDLL()
	{
		if (m_dllPath.empty())
			return;
		m_scriptEngine->FreeDLL();

		auto rootGameObject = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetRootGameObject();

		CppSer::Serializer serializer;
		for (auto& child : rootGameObject.lock()->GetAllChildren())
		{

		}

		if (!m_scriptEngine->LoadDLL(m_dllPath))
		{
			PrintError("Failed to load DLL: %s", m_dllPath.string().c_str());
			return;
		}

	}

	void* Scripting::ScriptEngine::GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName)
	{
		return m_scriptEngine->GetScriptVariable(scriptComponent, scriptName, variableName);
	}

	void Scripting::ScriptEngine::SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, void* value)
	{
		m_scriptEngine->SetScriptVariable(scriptComponent, scriptName, variableName, value);
	}

	std::unordered_map<std::string, Scripting::VariableInfo> Scripting::ScriptEngine::GetAllScriptVariablesInfo(const std::string& scriptName)
	{
		std::unordered_map<std::string, Scripting::VariableInfo> variables;
		for (auto& variable : m_scriptEngine->GetAllScriptVariablesInfo(scriptName))
		{
			variables[variable.first] = VariableInfo(variable.second.property);
		}
		return variables;
	}

}
