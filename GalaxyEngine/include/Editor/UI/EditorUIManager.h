#pragma once
#include "GalaxyAPI.h"

#include "Editor/UI/MainBar.h"
#include "Editor/UI/Hierarchy.h"
#include "Editor/UI/Inspector.h"
#include "Editor/UI/SceneWindow.h"
#include "Editor/UI/Console.h"
#include "Editor/UI/FileExplorer.h"
#include "Editor/UI/FileDialog.h"

namespace GALAXY::Editor::UI {
	class EditorUIManager
	{
	public:
		EditorUIManager();
		EditorUIManager& operator=(const EditorUIManager& other) = default;
		EditorUIManager(const EditorUIManager&) = default;
		EditorUIManager(EditorUIManager&&) noexcept = default;
		virtual ~EditorUIManager() {}

		static EditorUIManager* GetInstance();

		static void Initialize();

		static void Release();

		void DrawUI();

		static void DrawMainDock();

		// Safe popup to save scene if modified
		void DisplayClosePopup();
		void SetShouldDisplayClosePopup(bool shouldDisplay) { m_shouldDisplayClosePopup = shouldDisplay; }
		bool ShouldDisplaySafeClose();

		inline MainBar* GetMainBar() const { return m_mainBar.get(); }
		inline Hierarchy* GetHierarchy() const { return m_hierarchy.get(); }
		inline Inspector* GetInspector() const { return m_inspector.get(); }
		inline SceneWindow* GetSceneWindow() const { return m_sceneWindow.get(); }
		inline Console* GetConsole() const { return m_console.get(); }
		inline FileExplorer* GetFileExplorer() const { return m_fileExplorer.get(); }
		inline FileDialog* GetFileDialog() const { return m_fileDialog.get(); }
	private:
		static Unique<EditorUIManager> m_instance;
		Unique<MainBar> m_mainBar;
		Unique<Hierarchy> m_hierarchy;
		Unique<Inspector> m_inspector;
		Unique<SceneWindow> m_sceneWindow;
		Unique<Console> m_console;
		Unique<FileExplorer> m_fileExplorer;
		Unique<FileDialog> m_fileDialog;

		bool m_shouldDisplayClosePopup = false;
		std::optional<bool> m_shouldDisplaySafeClose;
	};
}
