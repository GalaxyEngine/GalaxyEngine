#include "pch.h"
#include "Scripting/ScriptEngine.h"
#include "Scripting/HeaderParser.h"
#include "Scripting/ScriptInstance.h"

#include "Resource/Script.h"

#define DESTINATION_DLL std::filesystem::path("")

namespace GALAXY 
{

	std::unique_ptr<Scripting::ScriptEngine> Scripting::ScriptEngine::m_instance;
	Scripting::ScriptEngine::ScriptEngine()
	{

	}

	Scripting::ScriptEngine::~ScriptEngine()
	{
	}


	void Scripting::ScriptEngine::LoadDLL(const std::filesystem::path& dllPath, const std::string& dllName)
	{
		auto dllPathName = dllPath / (dllName + ".dll");
		auto pdbPathName = dllPath / (dllName + ".pdb");

		if (!std::filesystem::exists(dllPathName))
		{
			PrintError("Failed to load project DLL.");
			return;
		}

		m_dllName = dllName;

		std::filesystem::path copiedDllPath = DESTINATION_DLL / (dllName + ".dll");
		std::filesystem::path copiedPdbPath = DESTINATION_DLL / (dllName + ".pdb");

		Utils::FileSystem::RemoveFile(copiedDllPath);
		Utils::FileSystem::RemoveFile(copiedPdbPath);

		Utils::FileSystem::CopyFileTo(dllPathName, copiedDllPath);
		Utils::FileSystem::CopyFileTo(pdbPathName, copiedPdbPath);

		m_hDll = LoadLibrary(copiedDllPath.string().c_str());
		if (m_hDll != NULL) {
			PrintLog("Loading Project %s", dllName.c_str());

			for (auto& script : m_scripts)
			{
				ParseScript(script);
			}
		}
		else {
			PrintError("Failed to load project DLL.");
		}
	}

	void Scripting::ScriptEngine::UnloadDLL()
	{
		if (m_dllLoaded && m_hDll != NULL)
		{
			FreeLibrary(m_hDll);
		}
	}

	void Scripting::ScriptEngine::AddScript(Weak<Resource::Script> script)
	{
		m_scripts.push_back(script);

		if (!m_dllLoaded)
			return;

		// Parse if dll already loaded
		ParseScript(script);
	}

	void Scripting::ScriptEngine::ParseScript(Weak<Resource::Script>& script)
	{
		auto properties = m_headerParser->ParseFile(script.lock()->GetFileInfo().GetFullPath());
		std::string className = script.lock()->GetName();
		className = className.substr(0, className.find_last_of('.'));
		auto scriptInstance = m_scriptInstances[className] = std::make_shared<ScriptInstance>();

		scriptInstance->m_constructor = GetConstructor(className);
		for (auto& property : properties)
		{
			auto type = StringToVariableType(property.propertyType);
			scriptInstance->m_gettersMethods[property.propertyName] = GetGetter(className, property.propertyName);
			scriptInstance->m_settersMethods[property.propertyName] = GetSetter(className, property.propertyName);
			scriptInstance->m_variables[property.propertyName] = type;
		}
	}

	Scripting::VariableType Scripting::ScriptEngine::StringToVariableType(const std::string& typeName)
	{
		if (typeName == "bool")
			return VariableType::Bool;
		else if (typeName == "int")
			return VariableType::Int;
		else if (typeName == "float")
			return VariableType::Float;
		else if (typeName == "double")
			return VariableType::Double;
		else if (typeName == "Vec2f" || typeName == "Math::Vec2f")
			return VariableType::Vector2;
		else if (typeName == "Vec3f" || typeName == "Math::Vec3f")
			return VariableType::Vector3;
		else if (typeName == "Vec4f" || typeName == "Math::Vec4f")
			return VariableType::Vector4;
		else 
			return VariableType::Unknown;
	}

	void* Scripting::ScriptEngine::GetVariableOfScript(Component::BaseComponent* component, const std::string& scriptName, const std::string& variableName)
	{
		if (m_scriptInstances.contains(scriptName) && m_scriptInstances[scriptName]->m_gettersMethods.contains(variableName))
			return m_scriptInstances[scriptName]->m_gettersMethods[variableName](component);
	}

	void Scripting::ScriptEngine::SetVariableOfScript(Component::BaseComponent* component, const std::string& scriptName, const std::string& variableName, void* value)
	{
		if (m_scriptInstances.contains(scriptName) && m_scriptInstances[scriptName]->m_settersMethods.contains(variableName))
			m_scriptInstances[scriptName]->m_settersMethods[variableName](component, value);
	}

	Scripting::ScriptConstructor Scripting::ScriptEngine::GetConstructor(const std::string& className)
	{
		return (ScriptConstructor)(GetProcAddress(m_hDll, "Create_TestScript"));
	}

	Scripting::GetterMethod Scripting::ScriptEngine::GetGetter(const std::string& className, const std::string& variableName)
	{
		return (GetterMethod)(GetProcAddress(m_hDll, ("Get_" + className + '_' + variableName).c_str()));
	}

	Scripting::SetterMethod Scripting::ScriptEngine::GetSetter(const std::string& className, const std::string& variableName)
	{
		return (SetterMethod)(GetProcAddress(m_hDll, ("Set_" + className + '_' + variableName).c_str()));
	}

}
