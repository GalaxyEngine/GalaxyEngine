#pragma once
#include "GalaxyAPI.h"
#include <unordered_map>
namespace GALAXY 
{
	namespace Component { class BaseComponent; class ScriptComponent; }
	namespace Resource { class Script; }
	namespace Utils { class FileWatcher; }
	namespace Scripting {

		using ScriptConstructor = void* (*)();
		using GetterMethod = void* (*)(void*);
		using SetterMethod = void (*)(void*, void*);

		class ScriptEngine
		{
		public:
			ScriptEngine();
			~ScriptEngine();

			static inline ScriptEngine* GetInstance();

			void LoadDLL(const std::filesystem::path& dllPath, const std::string& dllName);

			void UnloadDLL();

			void AddScript(Weak<Resource::Script> script);

			void ParseScript(Weak<Resource::Script>& script);

			void CleanScripts();

			void OnDLLUpdated();

			void ReloadDLL();

			void UpdateFileWatcherDLL();

			static enum class VariableType StringToVariableType(const std::string& typeName);

			Weak<class ScriptInstance> GetScriptInstance(const std::string& scriptName);

			Shared<Component::BaseComponent> CreateScript(const std::string& scriptName);

			void* GetVariableOfScript(Component::BaseComponent* component, const std::string& scriptName, const std::string& variableName);
			void SetVariableOfScript(Component::BaseComponent* component, const std::string& scriptName, const std::string& variableName, void* value);
		private:
			ScriptConstructor GetConstructor(const std::string& className);
			GetterMethod GetGetter(const std::string& className, const std::string& variableName);
			SetterMethod GetSetter(const std::string& className, const std::string& variableName);
		private:
			static Unique<ScriptEngine> m_instance;
			Shared<class HeaderParser> m_headerParser = nullptr;

			Shared<Utils::FileWatcher> m_fileWatcherDLL;

			HMODULE m_hDll = NULL;
			bool m_dllLoaded = false;

			std::filesystem::path m_dllPath;
			std::string m_dllName;

			bool m_shouldReloadDll = false;

			std::vector<Weak<Resource::Script>> m_scripts;
			std::vector<Component::ScriptComponent*> m_registeredScriptComponents;
			std::unordered_map<std::string, Shared<class ScriptInstance>> m_scriptInstances;
			std::vector<Weak<class ScriptComponent>> m_scriptComponents;
		};
	}
}
#include "Scripting/ScriptEngine.inl" 
