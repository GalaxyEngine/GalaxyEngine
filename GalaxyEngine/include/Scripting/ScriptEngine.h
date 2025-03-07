#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
#include <functional>
#include <unordered_map>

#include "Wrapper/GUI.h"

#include "Scripting/VariableInfo.h"
#include "Utils/Event.h"
#include "Utils/FileInfo.h"

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

			void Initialize();
			
			void RegisterScriptComponents() const;
			void UnregisterScriptComponents();

			void UpdateFileWatch();

			void FreeDLL();
			bool LoadDLL(const std::filesystem::path& dllPath);
			void ReloadDLL();
			void DeleteProjectDLL() const;

			// For file watcher to call
			void SetDLLPath(const std::filesystem::path& dllPath) { m_dllPath = dllPath; }

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
			static void CompileCode(bool force = false, bool monothread = false);

			static void GenerateSolution(Editor::ScriptEditorToolType tool);

#ifdef _WIN32
			static std::filesystem::path GetSLNPath();
#endif

			static void OpenSolution(Editor::ScriptEditorToolType tool);
#endif

			std::unordered_map<std::string, std::shared_ptr<Scripting::VariableInfo>> GetAllScriptVariablesInfo(const std::string& scriptName) const;

			static ScriptEngine* GetInstance();

			void ResetLastWriteTime();

			static std::filesystem::path GetFilePathForScript(const std::string& scriptClassName);

			Utils::Event<> EOnStartCompilation;
			Utils::Event<> EOnEndCompilation;
			Utils::Event<> EOnStartReloadLib;
			Utils::Event<> EOnEndReloadLib;
			Utils::Event<> EOnStartGenerateSolution;
			Utils::Event<> EOnEndGenerateSolution;
		private:
			static std::unique_ptr<ScriptEngine> s_instance;

			GS::ScriptEngine* m_engine = nullptr;

			std::filesystem::path m_dllPath;

			float m_currentTime = 0;
			const float m_updateInterval = 1.f;
			std::optional<std::filesystem::file_time_type> m_lastWriteTime;

			std::filesystem::path m_dllCopyToFolder;

		};
	}
}