#pragma once
#include "GalaxyAPI.h"
#include <optional>
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
		enum class EditorSettingsTab
		{
			General = 0,
			ExternalTool,
			Appearance,
			Benchmark,
		};

		inline const char* SerializeEditorSettingsTabValue(EditorSettingsTab tab)
		{
			switch (tab)
			{
			case EditorSettingsTab::General: return "General";
			case EditorSettingsTab::ExternalTool: return "External Tool";
			case EditorSettingsTab::Appearance:	return "Appearance";
			case EditorSettingsTab::Benchmark:	return "Benchmark";
			default: return "Invalid";
			}
		}

		enum class ScriptEditorTool
		{
			None = 0,
#ifdef _WIN32
			VisualStudio = 1,
#endif
			VisualStudioCode
		};

		inline const char* SerializeScriptEditorToolEnum()
		{
#ifdef _WIN32
			return "None\0Visual Studio\0Visual Studio Code\0";
#else
			return "None\0Visual Studio Code\0";
#endif
		}

		class EditorSettings
		{
		public:
			~EditorSettings();

			static EditorSettings& GetInstance();

			void Draw();

			void TakeScreenShot();

			void AddListElement(EditorSettingsTab tab);

			[[nodiscard]] ScriptEditorTool GetScriptEditorTool() const { return m_scriptEditorTool; }
			void SetScriptEditorTool(const ScriptEditorTool val) { m_scriptEditorTool = val; }

			void SaveSettings() const;
			void LoadSettings();
		private:
			void DisplayTab(EditorSettingsTab tab);

			void DisplayGeneralTab();
			void DisplayExternalToolTab();
			void DisplayAppearanceTab();
			void DisplayBenchmarkTab();

			void UpdateScreenShot();
		private:
			friend Core::Application;
			bool m_firstUpdate = false;
			bool m_shouldTakeScreenshot = false;

			EditorSettingsTab m_selectedTab = EditorSettingsTab::General;
#if defined(_WIN32)
			ScriptEditorTool m_scriptEditorTool = ScriptEditorTool::VisualStudio;
#else
			ScriptEditorTool m_scriptEditorTool = ScriptEditorTool::VisualStudioCode;
#endif
			Weak<Resource::Texture> m_projectThumbnail = {};
		};
	}
}
