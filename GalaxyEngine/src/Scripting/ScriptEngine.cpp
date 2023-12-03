#include "pch.h"
#include "Scripting/ScriptEngine.h"
#include "Scripting/HeaderParser.h"
#include "Scripting/ScriptInstance.h"

#include "Resource/Script.h"
#include "Resource/Scene.h"

#include "Component/ComponentHolder.h"
#include "Component/ScriptComponent.h"

#include "Utils/FileWatcher.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Core/ThreadManager.h"

#include "Utils/OS.h"

#define DESTINATION_DLL std::filesystem::path("")

namespace GALAXY
{

	std::unique_ptr<Scripting::ScriptEngine> Scripting::ScriptEngine::m_instance;

	Scripting::ScriptEngine::~ScriptEngine()
	{
		UnloadDLL();
	}

	void Scripting::ScriptEngine::CopyDLLFile(const std::filesystem::path& originalPath, const std::filesystem::path& copiedPath)
	{
		Utils::FileSystem::RemoveFile(copiedPath);
		Utils::FileSystem::CopyFileTo(originalPath, copiedPath);
		copiedFile++;
	}

	void Scripting::ScriptEngine::LoadDLL(const std::filesystem::path& dllPath, const std::string& dllName)
	{
		const std::string extension = Utils::OS::GetDLLExtension();
		#ifdef _WIN32
		auto dllPathName = dllPath / (dllName + extension);
		#else
		auto dllPathName = dllPath / ("lib" + dllName + extension);
		#endif
		auto pdbPathName = dllPath / (dllName + ".pdb");
		auto libPathName = dllPath / (dllName + ".lib");
		auto libPathName2 = dllPath / (dllName + ".dll.a");

		m_dllPath = dllPath;
		m_dllName = dllName;

		if (!m_fileWatcherDLL)
		{
			std::function<void()> func = std::bind(&ScriptEngine::OnDLLUpdated, this);
			m_fileWatcherDLL = std::make_shared<Utils::FileWatcher>(dllPathName.string(), func);
		}

		if (!std::filesystem::exists(dllPathName))
		{
			PrintError("Failed to load project DLL: file %s not exist", dllPathName.string().c_str());
			return;
		}

		#ifdef _WIN32
		std::filesystem::path copiedDllPath = DESTINATION_DLL / (dllName + extension);
		#else
		std::filesystem::path copiedDllPath = DESTINATION_DLL / ("lib" + dllName + extension);
		#endif
		std::filesystem::path copiedPdbPath = DESTINATION_DLL / (dllName + ".pdb");
		std::filesystem::path copiedLibPath = DESTINATION_DLL / (dllName + ".lib");
		std::filesystem::path copiedLib2Path = DESTINATION_DLL / (dllName + ".dll.a");

		bool shouldCopyFiles = true;
		if (std::filesystem::exists(copiedDllPath))
		{
			const auto lastTimeCopied = std::filesystem::last_write_time(copiedDllPath);
			const auto lastTimeDLL = std::filesystem::last_write_time(dllPathName);
			if (lastTimeCopied > lastTimeDLL)
			{
				shouldCopyFiles = false;
			}
		}

		if (shouldCopyFiles)
		{
			/*
			static auto threadManager = Core::ThreadManager::GetInstance();
			threadManager->AddTask(&Scripting::ScriptEngine::CopyDLLFile, this, dllPathName, copiedDllPath);
			threadManager->AddTask(&Scripting::ScriptEngine::CopyDLLFile, this, pdbPathName, copiedPdbPath);
			threadManager->AddTask(&Scripting::ScriptEngine::CopyDLLFile, this, libPathName, copiedLibPath);
			*/

			CopyDLLFile(dllPathName, copiedDllPath);
			CopyDLLFile(pdbPathName, copiedPdbPath);
			CopyDLLFile(libPathName, copiedLibPath);
			CopyDLLFile(libPathName, copiedLib2Path);

			/*
			while (copiedFile != 3)
			{
			}
			*/
			copiedFile = 0;
		}

		const std::string dllLoad = (std::filesystem::current_path() / copiedDllPath.string()).string();

		if (!std::filesystem::exists(dllLoad))
			return;

		m_hDll = Utils::OS::LoadDLL(dllLoad.c_str());

		if (m_hDll != nullptr)
		{
			m_dllLoaded = true;
			for (auto& script : m_scripts)
			{
				ParseScript(script);
			}
		}
	}


	void Scripting::ScriptEngine::UnloadDLL() const
	{
		if (m_dllLoaded && m_hDll != nullptr)
		{
			Utils::OS::FreeDLL(m_hDll);
		}
	}

	void Scripting::ScriptEngine::AddScript(const Weak<Resource::Script>& script)
	{
		m_scripts.push_back(script);

		if (!m_dllLoaded)
			return;

		ParseScript(script);
	}

	void Scripting::ScriptEngine::RemoveScript(const Weak<Resource::Script>& script)
	{
		for (size_t i = 0; i < m_scripts.size(); i++) {
			if (script.lock().get() == m_scripts[i].lock().get())
			{
				for (const auto scriptComponent : m_registeredScriptComponents)
				{
					const char* componentName = scriptComponent->GetComponentName();
					const std::string scriptName = script.lock()->GetFileInfo().GetFileNameNoExtension();
					if (componentName == scriptName)
					{
						Component::ComponentHolder::UnregisterComponent(scriptComponent);
						break;
					}
					}

				m_scripts.erase(m_scripts.begin() + i);
				return;
				}
			}
		}

	void Scripting::ScriptEngine::ParseScript(const Weak<Resource::Script>& script)
	{
		std::string className = script.lock()->GetName();
		className = className.substr(0, className.find_last_of('.'));

		// Get the constructor
		const Shared<ScriptInstance> scriptInstance = m_scriptInstances[className] = std::make_shared<ScriptInstance>();
		scriptInstance->m_constructor = GetConstructor(className);

		// Get the component and register it in componentHolder
		Component::ScriptComponent* component = reinterpret_cast<Component::ScriptComponent*>(scriptInstance->m_constructor());

		Component::ComponentHolder::RegisterComponent<Component::ScriptComponent>(component);
		m_registeredScriptComponents.push_back(component);

		// Parse script and get getters, setters and variables.
		const auto properties = m_headerParser->ParseFile(script.lock()->GetFileInfo().GetFullPath());
		for (auto& property : properties)
		{
			const auto type = StringToVariableType(property.propertyType);
			scriptInstance->m_gettersMethods[property.propertyName] = GetGetter(className, property.propertyName);
			scriptInstance->m_settersMethods[property.propertyName] = GetSetter(className, property.propertyName);
			scriptInstance->m_variables[property.propertyName].type = type;
			scriptInstance->m_variables[property.propertyName].typeName = property.propertyType;
			scriptInstance->m_variables[property.propertyName].isAList = property.isAList;
		}
	}

	void Scripting::ScriptEngine::CleanScripts()
	{
		// Unregister all script Components
		for (const auto scriptComponent : m_registeredScriptComponents)
		{
			Component::ComponentHolder::UnregisterComponent(scriptComponent);
		}
		m_scriptInstances.clear();
	}

	void Scripting::ScriptEngine::OnDLLUpdated()
	{
		PrintLog("Dll Updated");
		if (m_dllLoaded)
			ReloadDLL();
		else
			LoadDLL(m_dllPath, m_dllName);
	}

	void Scripting::ScriptEngine::ReloadDLL()
	{
		const Core::SceneHolder* sceneHolder = Core::SceneHolder::GetInstance();
		const Resource::Scene* currentScene = sceneHolder->GetCurrentScene();
		const Shared<Core::GameObject> rootGameObject = currentScene->GetRootGameObject().lock();

		const auto childGameObjects = rootGameObject->GetAllChildren();

		// Initialize vectors to store reloaders and component IDs for each child game object
		std::vector<std::vector<Component::ReloadScript>> reloaders(childGameObjects.size());
		std::vector<std::vector<uint32_t>> componentIDs(childGameObjects.size());

		for (size_t i = 0; i < childGameObjects.size(); ++i)
		{
			auto& currentReloaderList = reloaders[i];
			auto& currentComponentID = componentIDs[i];
			const auto childGameObject = childGameObjects[i].lock();

			if (!childGameObject)
				continue;

			// Get script components and create reloaders
			auto scriptComponents = childGameObject->GetComponentsPrivate<Component::ScriptComponent>();
			for (auto& component : scriptComponents)
			{
				if (component)
				{
					currentReloaderList.emplace_back(component);
					currentComponentID.push_back(component->GetIndex());

					// Reload the script and remove it from the game object
					auto& reload = currentReloaderList.back();
					reload.BeforeReloadScript();
					component->RemoveFromGameObject();
				}
			}
		}	

		// Clean up old scripts
		CleanScripts();

		// Unload the old DLL
		UnloadDLL();

		// Load the new DLL
		LoadDLL(m_dllPath, m_dllName);

		for (size_t i = 0; i < reloaders.size(); i++)
		{
			auto& currentReloaderList = reloaders[i];
			auto& currentComponentID = componentIDs[i];
			const auto& childGameObject = childGameObjects[i];

			for (size_t j = 0; j < currentReloaderList.size(); ++j)
			{
				// Check if the script still exists
				if (!ScriptExist(currentReloaderList[j].GetScriptName()))
					continue;

				// After reloading the script, add it back to the game object
				currentReloaderList[j].AfterReloadScript();
				childGameObject.lock()->AddComponent(currentReloaderList[j].GetComponent(), currentComponentID[j]);
			}
		}

		// Sync component values for all reloaders
		for (auto& currentReloaderList : reloaders)
		{
			for (const auto& j : currentReloaderList)
			{
				j.SyncComponentsValues();
			}
		}
	}

	void Scripting::ScriptEngine::UpdateFileWatcherDLL() const
	{
		if (m_fileWatcherDLL)
			m_fileWatcherDLL->Update();
	}

	Scripting::VariableType Scripting::ScriptEngine::StringToVariableType(const std::string& typeName)
	{
		if (typeMap.contains(typeName))
			return typeMap[typeName];
		else
		{
			// Check the names of all components to see if they match.
			if (typeName == "BaseComponent")
				return VariableType::Component;
			for (const auto& componentRegistered : Component::ComponentHolder::GetList())
			{
				if (componentRegistered->GetComponentName() == typeName)
				{
					return VariableType::Component;
				}
			}
			return VariableType::Unknown;
		}
	}

	Weak<Scripting::ScriptInstance> Scripting::ScriptEngine::GetScriptInstance(const char* scriptName)
	{
		if (m_scriptInstances.contains(scriptName))
			return m_scriptInstances.at(scriptName);
		return {};
	}

	Shared<Component::ScriptComponent> Scripting::ScriptEngine::CreateScript(const std::string& scriptName) const
	{
		if (m_scriptInstances.contains(scriptName))
		{
			const auto scriptInstance = m_scriptInstances.at(scriptName);
			auto component = reinterpret_cast<Component::ScriptComponent*>(scriptInstance->m_constructor());
			return Shared<Component::ScriptComponent>(component);
		}
		return nullptr;
	}

	void* Scripting::ScriptEngine::GetVariableOfScript(Component::ScriptComponent* component, const std::string& scriptName, const std::string& variableName)
	{
		if (m_scriptInstances.contains(scriptName)
			&& m_scriptInstances[scriptName]->m_gettersMethods.contains(variableName)
			&& m_scriptInstances[scriptName]->m_gettersMethods[variableName])
		{
			return m_scriptInstances[scriptName]->m_gettersMethods[variableName](component);
		}
		return nullptr;
	}

	void Scripting::ScriptEngine::SetVariableOfScript(Component::ScriptComponent* component, const std::string& scriptName, const std::string& variableName, void* value)
	{
		if (m_scriptInstances.contains(scriptName) && m_scriptInstances[scriptName]->m_settersMethods.contains(variableName))
			m_scriptInstances[scriptName]->m_settersMethods[variableName](component, value);
	}

	Scripting::ScriptConstructor Scripting::ScriptEngine::GetConstructor(const std::string& className) const
	{
		return Utils::OS::GetDLLMethod<ScriptConstructor>(m_hDll, ("Create_" + className).c_str());
	}

	Scripting::GetterMethod Scripting::ScriptEngine::GetGetter(const std::string& className, const std::string& variableName) const
	{
		const std::string getterMethodName = ("Get_" + className + '_' + variableName).c_str();
		return Utils::OS::GetDLLMethod<GetterMethod>(m_hDll, getterMethodName.c_str());
	}

	Scripting::SetterMethod Scripting::ScriptEngine::GetSetter(const std::string& className, const std::string& variableName) const
	{
		const std::string setterMethodName = ("Set_" + className + '_' + variableName).c_str();
		return Utils::OS::GetDLLMethod<SetterMethod>(m_hDll, setterMethodName.c_str());
	}

	bool Scripting::ScriptEngine::ScriptExist(const std::string& scriptName) const
	{
		for (const auto& m_script : m_scripts)
		{
			if (scriptName == m_script.lock()->GetFileInfo().GetFileNameNoExtension())
			{
				return m_script.lock() != nullptr;
			}
	}
		return false;
}

}
