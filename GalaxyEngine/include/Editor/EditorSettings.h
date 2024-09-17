#pragma once
#include "GalaxyAPI.h"
#include <optional>
#include <map>
#include <string>

#include "Core/Input.h"
#include "Utils/FileInfo.h"


namespace GALAXY 
{
	namespace Core
	{
		class Application;
	}
	namespace Resource
	{
		class Texture;
	}
	namespace Editor
	{
		enum class ApplicationMode
		{
			Play = 0,
			Pause,
			Editor,
		};
		
		enum class EditorSettingsTab
		{
			General = 0,
			ExternalTool,
			Appearance,
			Benchmark,
			EditorInputs,
		};

		inline const char* SerializeEditorSettingsTabValue(EditorSettingsTab tab)
		{
			switch (tab)
			{
			case EditorSettingsTab::General: return "General";
			case EditorSettingsTab::ExternalTool: return "External Tool";
			case EditorSettingsTab::Appearance:	return "Appearance";
			case EditorSettingsTab::Benchmark:	return "Benchmark";
			case EditorSettingsTab::EditorInputs:	return "Editor Inputs";
			default: return "Invalid";
			}
		}

		enum class ScriptEditorTool
		{
			None = 0,
			VisualStudioCode = 1,
#ifdef _WIN32
			VisualStudio = 2,
			Rider = 3,
#endif
			Custom = 4
		};

		inline const char* SerializeScriptEditorToolValue(ScriptEditorTool tool)
		{
			switch (tool)
			{
			case ScriptEditorTool::None: return "None";
			case ScriptEditorTool::VisualStudioCode: return "Visual Studio Code";
#ifdef _WIN32
			case ScriptEditorTool::VisualStudio: return "Visual Studio";
			case ScriptEditorTool::Rider: return "Rider";
#endif
			case ScriptEditorTool::Custom: return "Custom";
			default: return "Invalid";
			}
		}

		enum class InputAction
		{
			None = 0,
			Forward,
			Backward,
			Left,
			Right,
			Up,
			Down,
			FastMode,
		};

		struct EditorInput
		{
			std::string name;
			Key key;

			EditorInput(const std::string& name, const Key key) : name(name), key(key) {}
			EditorInput() = default;
		};

		class EditorInputsManager
		{
		public:
			Key GetInputForAction(const InputAction action) const
			{
				return EditorInputs.at(action).key;
			}
			
			std::unordered_map<InputAction, EditorInput> EditorInputs;

			void Initialize();
		};
		

		class EditorSettings
		{
		public:
			EditorSettings();
			~EditorSettings();

			static EditorSettings& GetInstance();
			
			void Display();

			void TakeScreenShot();

			void AddListElement(EditorSettingsTab tab);

			[[nodiscard]] ScriptEditorTool GetScriptEditorTool() const { return m_scriptEditorTool; }
			void SetScriptEditorTool(const ScriptEditorTool val) { m_scriptEditorTool = val; }
			void OpenWithScriptEditorTool(const Path& path);

			void SaveSettings() const;
			void LoadSettings();

			void InitializeScriptEditorTools();

			[[nodiscard]] Path GetOtherScriptEditorToolPath() const { return m_otherScriptEditorToolPath.value(); }

			[[nodiscard]] EditorInputsManager& GetEditorInputsManager() { return m_editorInputsManager; }
		private:
			void DisplayTab(EditorSettingsTab tab);

			void DisplayGeneralTab();
			
			void DisplayExternalToolTab();
			void ChangeOtherScriptTool();
			
			void DisplayAppearanceTab();
			
			void DisplayBenchmarkTab();

			void DisplayEditorInputsTab();

			void UpdateScreenShot();
		private:
			friend Core::Application;
			bool m_firstUpdate = false;
			bool m_shouldTakeScreenshot = false;

			EditorSettingsTab m_selectedTab = EditorSettingsTab::General;

#pragma region External Tools
			std::optional<Path> m_otherScriptEditorToolPath = std::nullopt;

			std::map<ScriptEditorTool, std::string> m_scriptEditorToolsString = {}; 
#if defined(_WIN32)
			ScriptEditorTool m_scriptEditorTool = ScriptEditorTool::VisualStudio;
#else
			ScriptEditorTool m_scriptEditorTool = ScriptEditorTool::VisualStudioCode;
#endif
#pragma endregion

			EditorInputsManager m_editorInputsManager;
			
			Weak<Resource::Texture> m_projectThumbnail = {};
		};
	}
}
