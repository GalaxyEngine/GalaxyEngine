#pragma once
#include "GalaxyAPI.h"
#include <filesystem>

#include <atomic>

namespace GALAXY
{
	namespace Component { class BaseComponent; class ScriptComponent; }
	namespace Resource { class Script; }
	namespace Utils { class FileWatcher; }
	namespace Scripting {
		enum class VariableType
		{
			Unknown,
			Bool,
			Int,
			Float,
			Double,
			Vector2,
			Vector3,
			Vector4,
			String,
			Component,
			GameObject,
		};

		static UMap<std::string, VariableType> typeMap = 
		{
			{"Unknown", VariableType::Unknown},
			{"bool", VariableType::Bool},
			{"int", VariableType::Int},
			{"float", VariableType::Float},
			{"double", VariableType::Double},
			{"Vec2f", VariableType::Vector2},
			{"Vec3f", VariableType::Vector3},
			{"Vec4f", VariableType::Vector4},
			{"string", VariableType::String},
			{"BaseComponent", VariableType::Component},
			{"GameObject", VariableType::GameObject}
		};

		struct GALAXY_API VariableData
		{
			VariableType type = VariableType::Unknown;
			std::string typeName = "";
			bool isAList = false;
		};

		using ScriptConstructor = Component::BaseComponent* (*)();
		using GetterMethod = void* (*)(void*);
		using SetterMethod = void (*)(void*, void*);

		class GALAXY_API ScriptEngine
		{
		public:
			~ScriptEngine();

			static inline ScriptEngine* GetInstance();

			void LoadDLL(const std::filesystem::path& dllPath, const std::string& dllName);

			void UnloadDLL() const;

			void AddScript(const Weak<Resource::Script>& script);

			void RemoveScript(const Weak<Resource::Script>& script);

			void ParseScript(const Weak<Resource::Script>& script);

			void CleanScripts();

			void OnDLLUpdated();

			void ReloadDLL();

			void UpdateFileWatcherDLL() const;

			static VariableType StringToVariableType(const std::string& typeName);

			Weak<class ScriptInstance> GetScriptInstance(const char* scriptName);

			Shared<Component::ScriptComponent> CreateScript(const std::string& scriptName) const;

			void* GetVariableOfScript(Component::ScriptComponent* component, const std::string& scriptName, const std::string& variableName);
			void SetVariableOfScript(Component::ScriptComponent* component, const std::string& scriptName, const std::string& variableName, void* value);

		private:
			ScriptConstructor GetConstructor(const std::string& className) const;
			GetterMethod GetGetter(const std::string& className, const std::string& variableName) const;
			SetterMethod GetSetter(const std::string& className, const std::string& variableName) const;

			bool ScriptExist(const std::string& scriptName) const;

			void CopyDLLFile(const std::filesystem::path& originalPath, const std::filesystem::path& copiedPath);
		private:
			static Unique<ScriptEngine> m_instance;
			Shared<class HeaderParser> m_headerParser = nullptr;

			Shared<Utils::FileWatcher> m_fileWatcherDLL;

			void* m_hDll = nullptr;

			bool m_dllLoaded = false;

			std::filesystem::path m_dllPath;
			std::string m_dllName;

			bool m_shouldReloadDll = false;

			std::vector<Weak<Resource::Script>> m_scripts;
			std::vector<Component::ScriptComponent*> m_registeredScriptComponents;
			std::unordered_map<std::string, Shared<class ScriptInstance>> m_scriptInstances;
			std::vector<Weak<class ScriptComponent>> m_scriptComponents;

			std::atomic<int> copiedFile = 0;
		};
	}
}
#include "Scripting/ScriptEngine.inl" 
