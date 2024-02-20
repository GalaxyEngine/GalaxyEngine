#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
#include <functional>
#include <unordered_map>

#include <Wrapper/GUI.h>

#include <Scripting/VariableInfo.h>

namespace GS { class ScriptEngine; struct Property; }
namespace GALAXY
{
	namespace Scripting
	{
		class ScriptEngine
		{
		public:
			ScriptEngine();
			~ScriptEngine();
			void RegisterScriptComponents();
			void UnregisterScriptComponents();

			void UpdateFileWatch() {} // TODO

			void FreeDLL();

			void LoadDLL(const std::filesystem::path& dllPath);

			void ReloadDLL(); //TODO : Do all the work of hot reload

			void* GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName);
			void SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, void* value);

			template<typename T>
			T* GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName)
			{
				return reinterpret_cast<T*>(GetScriptVariable(scriptComponent, scriptName, variableName));
			}

			template<typename T>
			void SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, T* value)
			{
				SetScriptVariable(scriptComponent, scriptName, variableName, reinterpret_cast<void*>(value));
			}

			std::unordered_map<std::string, std::shared_ptr<Scripting::VariableInfo>> GetAllScriptVariablesInfo(const std::string& scriptName);

			static ScriptEngine* GetInstance();
		private:
			static std::unique_ptr<ScriptEngine> s_instance;

			GS::ScriptEngine* m_scriptEngine = nullptr;

			std::filesystem::path m_dllPath;
		};
	}
}