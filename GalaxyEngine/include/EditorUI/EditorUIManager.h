#pragma once
#include "GalaxyAPI.h"

#include "EditorUI/MainBar.h"
#include "EditorUI/Hierarchy.h"
#include "EditorUI/Inspector.h"
#include "EditorUI/SceneWindow.h"
#include "EditorUI/Console.h"
#include "EditorUI/FileExplorer.h"

namespace GALAXY::EditorUI {
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

		void Release();

		void DrawUI();

		void DrawMainDock();

		inline MainBar* GetMainBar() const { return m_mainBar.get(); }
		inline Hierarchy* GetHierarchy() const { return m_hierarchy.get(); }
		inline Inspector* GetInspector() const { return m_inspector.get(); }
		inline SceneWindow* GetSceneWindow() const { return m_sceneWindow.get(); }
		inline Console* GetConsole() const { return m_console.get(); }
		inline FileExplorer* GetFileExplorer() const { return m_fileExplorer.get(); }
	private:
		static Unique<EditorUIManager> m_instance;
		Unique<MainBar> m_mainBar;
		Unique<Hierarchy> m_hierarchy;
		Unique<Inspector> m_inspector;
		Unique<SceneWindow> m_sceneWindow;
		Unique<Console> m_console;
		Unique<FileExplorer> m_fileExplorer;
	};
}
