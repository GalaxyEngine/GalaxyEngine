#include "pch.h"
#include "Scripting/ScriptEngine.h"
#include "Scripting/HeaderParser.h"
#include "Scripting/ScriptInstance.h"

#include "Resource/Script.h"

#include "Component/ComponentHolder.h"
#include "Component/ScriptComponent.h"

#include "Utils/FileWatcher.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Core/Scene.h"

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

	void OnDllUpdate()
	{
		PrintLog("Dll Updated !");
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

		m_dllPath = dllPath;
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
			m_dllLoaded = true;
		}
		else {
			PrintError("Failed to load project DLL.");
		}

		//if (m_fileWatcherDLL)
			//m_fileWatcherDLL->StopWatching();
		if (!m_fileWatcherDLL)
		{
			std::function<void()> func = std::bind(&ScriptEngine::OnDLLUpdated, this);
			m_fileWatcherDLL = std::make_shared<Utils::FileWatcher>(dllPathName.string(), func);
		}
		//m_fileWatcherDLL->StartWatching();
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

		Component::ScriptComponent* scriptComp = new Component::ScriptComponent();

		Component::BaseComponent* component = reinterpret_cast<Component::BaseComponent*>(scriptInstance->m_constructor());
		std::shared_ptr<Component::BaseComponent> shared = std::shared_ptr<Component::BaseComponent>(component);
		scriptComp->SetScriptComponent(shared);
		Component::ComponentHolder::RegisterComponent<Component::ScriptComponent>(scriptComp);
		m_registeredScriptComponents.push_back(scriptComp);

		for (auto& property : properties)
		{
			auto type = StringToVariableType(property.propertyType);
			scriptInstance->m_gettersMethods[property.propertyName] = GetGetter(className, property.propertyName);
			scriptInstance->m_settersMethods[property.propertyName] = GetSetter(className, property.propertyName);
			scriptInstance->m_variables[property.propertyName] = type;
		}
	}

	void Scripting::ScriptEngine::CleanScripts()
	{
		for (auto scriptComponent : m_registeredScriptComponents)
		{
			Component::ComponentHolder::UnregisterComponent(scriptComponent);
		}
		m_scriptInstances.clear();
	}

	void Scripting::ScriptEngine::OnDLLUpdated()
	{
		PrintLog("Dll Updated");
		ReloadDLL();
	}

	void Scripting::ScriptEngine::ReloadDLL()
	{
		auto components = Core::SceneHolder::GetInstance()->GetCurrentScene()->GetRootGameObject().lock()->GetComponentsInChildren<Component::ScriptComponent>();
		for (auto& component : components)
		{
			if (component.lock())
			{
				component.lock()->BeforeReloadScript();
			}
		}

		UnloadDLL();

		CleanScripts();

		LoadDLL(m_dllPath, m_dllName);

		for (auto& component : components)
		{
			if (component.lock())
			{
				component.lock()->AfterReloadScript();
			}
		}
	}

	void Scripting::ScriptEngine::UpdateFileWatcherDLL()
	{
		m_fileWatcherDLL->Update();
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

	Weak<Scripting::ScriptInstance> Scripting::ScriptEngine::GetScriptInstance(const std::string& scriptName)
	{
		if (m_scriptInstances.contains(scriptName))
			return m_scriptInstances.at(scriptName);
		return Weak< ScriptInstance>();
	}

	Shared<Component::BaseComponent> Scripting::ScriptEngine::CreateScript(const std::string& scriptName)
	{
		if (m_scriptInstances.contains(scriptName))
		{
			auto scriptInstance = m_scriptInstances.at(scriptName);
			return Shared<Component::BaseComponent>(reinterpret_cast<Component::BaseComponent*>(scriptInstance->m_constructor()));
		}
		return nullptr;
	}

	void* Scripting::ScriptEngine::GetVariableOfScript(Component::BaseComponent* component, const std::string& scriptName, const std::string& variableName)
	{
		if (m_scriptInstances.contains(scriptName)
			&& m_scriptInstances[scriptName]->m_gettersMethods.contains(variableName)
			&& m_scriptInstances[scriptName]->m_gettersMethods[variableName])
		{
			return m_scriptInstances[scriptName]->m_gettersMethods[variableName](component);
		}
		return nullptr;
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
		const char* getterMethodName = ("Get_" + className + '_' + variableName).c_str();
		return (GetterMethod)(GetProcAddress(m_hDll, getterMethodName));
	}

	Scripting::SetterMethod Scripting::ScriptEngine::GetSetter(const std::string& className, const std::string& variableName)
	{
		const char* setteMethodName = ("Set_" + className + '_' + variableName).c_str();
		return (SetterMethod)(GetProcAddress(m_hDll, setteMethodName));
	}

}
