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

		void Initialize();

		void DrawUI();

		void DrawMainDock();

		MainBar* GetMainBar() const { return m_mainBar.get(); }
		Hierarchy* GetHierarchy() const { return m_hierarchy.get(); }
		Inspector* GetInspector() const { return m_inspector.get(); }
		SceneWindow* GetSceneWindow() const { return m_sceneWindow.get(); }
		Console* GetConsole() const { return m_console.get(); }
		FileExplorer* GetFileExplorer() const { return m_fileExplorer.get(); }
	private:
		static std::unique_ptr<EditorUIManager> m_instance;
		std::unique_ptr<MainBar> m_mainBar;
		std::unique_ptr<Hierarchy> m_hierarchy;
		std::unique_ptr<Inspector> m_inspector;
		std::unique_ptr<SceneWindow> m_sceneWindow;
		std::unique_ptr<Console> m_console;
		std::unique_ptr<FileExplorer> m_fileExplorer;
	};
}
