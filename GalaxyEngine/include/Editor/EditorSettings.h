#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Editor
	{
		enum class EditorSettingsTab
		{
			General = 0,
			ExternalTool,
			Appearance,
		};

		inline const char* SerializeEditorSettingsTabValue(EditorSettingsTab tab)
		{
			switch (tab)
			{
			case EditorSettingsTab::General:
				return "General";
			case EditorSettingsTab::ExternalTool:
				return "External Tool";
			case EditorSettingsTab::Appearance:
				return "Appearance";
			default:
				return "Invalid";
			}
		}

		enum class ScriptEditorTool
		{
			None = 0,
#ifdef _WIN32
			VisualStudio = 1,
#endif
			VisualStudioCode = 2
		};

		inline const char* SerializeScriptEditorToolEnum()
		{
#ifdef _WIN32
			return "None\0Visual Studio\0Visual Studio Code";
#else
			return "None\0Visual Studio Code";
#endif
		}

		//TODO: load from file
		class EditorSettings
		{
		public:
			~EditorSettings() {}

			static EditorSettings& GetInstance();

			void Draw();

			void AddListElement(EditorSettingsTab tab);

			void DisplayTab(EditorSettingsTab tab);
			void DisplayExternalToolTab();
			void DisplayAppearanceTab();

			ScriptEditorTool GetScriptEditorTool() const { return m_scriptEditorTool; }
			void SetScriptEditorTool(ScriptEditorTool val) { m_scriptEditorTool = val; }

			void SaveSettings();
			void LoadSettings();

		private:
			bool m_firstUpdate = false;

			EditorSettingsTab m_selectedTab = EditorSettingsTab::General;
#if defined(_WIN32)
			ScriptEditorTool m_scriptEditorTool = ScriptEditorTool::VisualStudio;
#else
			ScriptEditorTool m_scriptEditorTool = ScriptEditorTool::VisualStudioCode;
#endif
		};
	}
}
