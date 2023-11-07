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
#include "Resource/Scene.h"
#include "Core/ThreadManager.h"

#ifdef __linux__
#include <dlfcn.h>
#endif

#define DESTINATION_DLL std::filesystem::path("")

namespace GALAXY
{

	std::unique_ptr<Scripting::ScriptEngine> Scripting::ScriptEngine::m_instance;
	Scripting::ScriptEngine::ScriptEngine()
	{

	}

	Scripting::ScriptEngine::~ScriptEngine()
	{
		UnloadDLL();
	}

	void OnDllUpdate()
	{
		PrintLog("Dll Updated !");
	}

	std::string GetLastErrorAsString()
	{
		std::string message = "";
#ifdef _WIN32
		//Get the error message ID, if any.
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0) {
			return std::string(); //No error message has been recorded
		}

		LPSTR messageBuffer = nullptr;

		//Ask Win32 to give us the string version of that message ID.
		//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		//Copy the error message into a std::string.
		message = std::string(messageBuffer, size);

		//Free the Win32's string's buffer.
		LocalFree(messageBuffer);
#endif

		return message;
	}

	void Scripting::ScriptEngine::CopyDLLFile(const std::filesystem::path& originalPath, const std::filesystem::path& copiedPath)
	{
		Utils::FileSystem::RemoveFile(copiedPath);
		Utils::FileSystem::CopyFileTo(originalPath, copiedPath);
		copiedFile++;
	}

	void Scripting::ScriptEngine::LoadDLL(const std::filesystem::path& dllPath, const std::string& dllName)
	{
		std::string extension = "";
#if defined(_WIN32)
		extension = ".dll";
#elif defined(__linux__)
		extension = ".so";
#endif

		auto dllPathName = dllPath / (dllName + extension);
		auto pdbPathName = dllPath / (dllName + ".pdb");
		auto libPathName = dllPath / (dllName + ".lib");

		m_dllPath = dllPath;
		m_dllName = dllName;

		if (!m_fileWatcherDLL)
		{
			std::function<void()> func = std::bind(&ScriptEngine::OnDLLUpdated, this);
			m_fileWatcherDLL = std::make_shared<Utils::FileWatcher>(dllPathName.string(), func);
		}

		if (!std::filesystem::exists(dllPathName))
		{
			PrintError("Failed to load project DLL: file not %s exist", dllPathName.string().c_str());
			return;
		}

		std::filesystem::path copiedDllPath = DESTINATION_DLL / (dllName + extension);
		std::filesystem::path copiedPdbPath = DESTINATION_DLL / (dllName + ".pdb");
		std::filesystem::path copiedLibPath = DESTINATION_DLL / (dllName + ".lib");

		bool shouldCopyFiles = true;
		if (std::filesystem::exists(copiedDllPath))
		{
			auto lastTimeCopied = std::filesystem::last_write_time(copiedDllPath);
			auto lastTimeDLL = std::filesystem::last_write_time(dllPathName);
			if (lastTimeCopied > lastTimeDLL)
			{
				shouldCopyFiles = false;
			}
		}

		if (shouldCopyFiles)
		{
			auto threadManager = Core::ThreadManager::GetInstance();
			threadManager->AddTask(&Scripting::ScriptEngine::CopyDLLFile, this, dllPathName, copiedDllPath);
			threadManager->AddTask(&Scripting::ScriptEngine::CopyDLLFile, this, pdbPathName, copiedPdbPath);
			threadManager->AddTask(&Scripting::ScriptEngine::CopyDLLFile, this, libPathName, copiedLibPath);

			while (copiedFile != 3)
			{
			}
			copiedFile = 0;
		}

		auto dllLoad = copiedDllPath.string();
		bool loaded = false;

#if defined(_WIN32)
		m_hDll = LoadLibrary(dllLoad.c_str());

		if (m_hDll != NULL)
		{
			PrintLog("Loading Project %s", dllName.c_str());

			for (auto& script : m_scripts)
			{
				ParseScript(script);
			}
			m_dllLoaded = true;
			PrintLog("Loaded Project %s", dllName.c_str());
		}
		else
		{
			PrintError("Failed to load project DLL : %s", GetLastErrorAsString().c_str());
		}
#elif defined(__linux__)
		// Linux-specific code
		m_hDll = dlopen(dllLoad.c_str(), RTLD_LAZY);
		if (m_hDll)
		{
			loaded = true;
		}
		else
		{
			PrintError("Failed to load project DLL: %s", dlerror());
		}
#endif
	}


	void Scripting::ScriptEngine::UnloadDLL()
	{
		if (m_dllLoaded && m_hDll != NULL)
		{
#if defined(_WIN32)
			FreeLibrary(m_hDll);
#elif defined(__linux__)
			dlclose(m_hDll);
#endif
		}
	}

	void Scripting::ScriptEngine::AddScript(Weak<Resource::Script> script)
	{
		m_scripts.push_back(script);

		if (!m_dllLoaded)
			return;

		ParseScript(script);
	}

	void Scripting::ScriptEngine::RemoveScript(Weak<Resource::Script> script)
	{
		for (int i = 0; i < m_scripts.size(); i++) {
			if (script.lock().get() == m_scripts[i].lock().get())
			{
				for (auto scriptComponent : m_registeredScriptComponents)
				{
					const char* componentName = scriptComponent->GetComponentName();
					std::string scriptName = script.lock()->GetFileInfo().GetFileNameNoExtension();
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

	void Scripting::ScriptEngine::ParseScript(Weak<Resource::Script>& script)
	{
		std::string className = script.lock()->GetName();
		className = className.substr(0, className.find_last_of('.'));

		// Get the constructor
		Shared<ScriptInstance> scriptInstance = m_scriptInstances[className] = std::make_shared<ScriptInstance>();
		scriptInstance->m_constructor = GetConstructor(className);

		// Get the component and register it in componentHolder
		Component::ScriptComponent* component = reinterpret_cast<Component::ScriptComponent*>(scriptInstance->m_constructor());

		Component::ComponentHolder::RegisterComponent<Component::ScriptComponent>(component);
		m_registeredScriptComponents.push_back(component);

		// Parse script and get getters, setters and variables.
		auto properties = m_headerParser->ParseFile(script.lock()->GetFileInfo().GetFullPath());
		for (auto& property : properties)
		{
			auto type = StringToVariableType(property.propertyType);
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
		for (auto scriptComponent : m_registeredScriptComponents)
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
		Core::SceneHolder* sceneHolder = Core::SceneHolder::GetInstance();
		Resource::Scene* currentScene = sceneHolder->GetCurrentScene();
		Shared<Core::GameObject> rootGameObject = currentScene->GetRootGameObject().lock();

		auto childGameObjects = rootGameObject->GetAllChildren();

		// Initialize vectors to store reloaders and component IDs for each child game object
		std::vector<std::vector<Component::ReloadScript>> reloaders(childGameObjects.size());
		std::vector<std::vector<uint32_t>> componentIDs(childGameObjects.size());

		for (size_t i = 0; i < childGameObjects.size(); ++i)
		{
			auto& currentReloaderList = reloaders[i];
			auto& currentComponentID = componentIDs[i];
			auto childGameObject = childGameObjects[i].lock();

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

		for (int i = 0; i < reloaders.size(); i++)
		{
			auto& currentReloaderList = reloaders[i];
			auto& currentComponentID = componentIDs[i];
			auto childGameObject = childGameObjects[i];

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
		for (size_t i = 0; i < reloaders.size(); ++i)
		{
			auto& currentReloaderList = reloaders[i];
			for (size_t j = 0; j < currentReloaderList.size(); ++j)
			{
				currentReloaderList[j].SyncComponentsValues();
			}
		}
	}

	void Scripting::ScriptEngine::UpdateFileWatcherDLL()
	{
		if (m_fileWatcherDLL)
			m_fileWatcherDLL->Update();
	}

	Scripting::VariableType Scripting::ScriptEngine::StringToVariableType(const std::string& typeName)
	{
		if (typeMap.count(typeName))
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
		return Weak<ScriptInstance>();
	}

	Shared<Component::ScriptComponent> Scripting::ScriptEngine::CreateScript(const std::string& scriptName)
	{
		if (m_scriptInstances.contains(scriptName))
		{
			auto scriptInstance = m_scriptInstances.at(scriptName);
			Component::ScriptComponent* component = reinterpret_cast<Component::ScriptComponent*>(scriptInstance->m_constructor());
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

	Scripting::ScriptConstructor Scripting::ScriptEngine::GetConstructor(const std::string& className)
	{
#ifdef _WIN32
		return (ScriptConstructor)(GetProcAddress(m_hDll, ("Create_" + className).c_str()));
#elif defined(__linux__)
		return (ScriptConstructor)dlsym(m_hDll, ("Create_" + className).c_str());
#endif
	}

	Scripting::GetterMethod Scripting::ScriptEngine::GetGetter(const std::string& className, const std::string& variableName)
	{
		std::string getterMethodName = ("Get_" + className + '_' + variableName).c_str();
#ifdef _WIN32
		return (GetterMethod)(GetProcAddress(m_hDll, getterMethodName.c_str()));
#elif defined(__linux__)
		return (GetterMethod)dlsym(m_hDll, getterMethodName.c_str());
#endif
	}

	Scripting::SetterMethod Scripting::ScriptEngine::GetSetter(const std::string& className, const std::string& variableName)
	{
		std::string setterMethodName = ("Set_" + className + '_' + variableName).c_str();
#ifdef _WIN32
		return (SetterMethod)(GetProcAddress(m_hDll, setterMethodName.c_str()));
#elif defined(__linux__)
		return (SetterMethod)dlsym(m_hDll, setterMethodName.c_str());
#endif
	}

	bool Scripting::ScriptEngine::ScriptExist(const std::string& scriptName)
	{
		for (int i = 0; i < m_scripts.size(); i++)
		{
			if (scriptName == m_scripts[i].lock()->GetFileInfo().GetFileNameNoExtension())
			{
				return m_scripts[i].lock() != nullptr;
			}
		}
		return false;
	}

}
