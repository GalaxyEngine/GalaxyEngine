#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
#include <functional>
#include <unordered_map>

#include "Wrapper/GUI.h"

#include "Scripting/VariableInfo.h"

namespace GS { class ScriptEngine; struct Property; }
namespace GALAXY
{
	namespace Editor { enum class ScriptEditorToolType; }
	namespace Scripting
	{
		class ScriptEngine
		{
		public:
			ScriptEngine();
			~ScriptEngine();
			void RegisterScriptComponents();
			void UnregisterScriptComponents();

			void UpdateFileWatch();

			void FreeDLL();

			bool LoadDLL(const std::filesystem::path& dllPath);

			void ReloadDLL();

			void* GetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName) const;
			void SetScriptVariable(void* scriptComponent, const std::string& scriptName, const std::string& variableName, void* value) const;

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

#ifdef WITH_EDITOR
			static void CompileCode();

			static void GenerateSolution(Editor::ScriptEditorToolType tool);

#ifdef _WIN32
			static std::filesystem::path GetSLNPath();
#endif

			static void OpenSolution(Editor::ScriptEditorToolType tool);
#endif

			std::unordered_map<std::string, std::shared_ptr<Scripting::VariableInfo>> GetAllScriptVariablesInfo(const std::string& scriptName);

			static ScriptEngine* GetInstance();

			void ResetLastWriteTime();

			static std::filesystem::path GetFilePathForScript(const std::string& scriptClassName);
		private:
			static std::unique_ptr<ScriptEngine> s_instance;

			GS::ScriptEngine* m_scriptEngine = nullptr;

			std::filesystem::path m_dllPath;

			float m_currentTime = 0;
			const float m_updateInterval = 1.f;
			std::optional<std::filesystem::file_time_type> m_lastWriteTime;

		};
	}
}