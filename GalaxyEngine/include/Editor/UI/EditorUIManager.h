#pragma once
#include "GalaxyAPI.h"

#include "Editor/UI/MainBar.h"
#include "Editor/UI/Hierarchy.h"
#include "Editor/UI/Inspector.h"
#include "Editor/UI/SceneWindow.h"
#include "Editor/UI/Console.h"
#include "Editor/UI/FileExplorer.h"
#include "Editor/UI/ResourceWindow.h"
#include "Editor/UI/GameWindow.h"

#include <optional>

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
		void BindEvents();

		static void Release();

		void DrawUI();

		void MoveWindowCallback(const Vec2i& pos);

		// Scale the UI based on the current DPI, called before each frame
		void UpdateDPIScale();

		static void DrawMainDock();

		// Safe popup to save scene if modified
		void DisplayClosePopup();
		void SetShouldDisplayClosePopup(bool shouldDisplay) { m_shouldDisplayClosePopup = shouldDisplay; }
		bool ShouldDisplaySafeClose();

		inline MainBar* GetMainBar() const { return m_mainBar.get(); }
		inline Hierarchy* GetHierarchy() const { return m_hierarchy.get(); }
		inline Inspector* GetInspector() const { return m_inspector.get(); }
		inline SceneWindow* GetSceneWindow() const { return m_sceneWindow.get(); }
		inline GameWindow* GetGameWindow() const { return m_gameWindow.get(); }
		inline Console* GetConsole() const { return m_console.get(); }
		inline FileExplorer* GetFileExplorer() const { return m_fileExplorer.get(); }
		inline ResourceWindow* GetResourceWindow() const { return m_resourceWindow.get(); }
	private:
		static Unique<EditorUIManager> m_instance;
		Unique<MainBar> m_mainBar;
		Unique<Hierarchy> m_hierarchy;
		Unique<Inspector> m_inspector;
		Unique<SceneWindow> m_sceneWindow;
		Unique<GameWindow> m_gameWindow;
		Unique<Console> m_console;
		Unique<FileExplorer> m_fileExplorer;
		Unique<ResourceWindow> m_resourceWindow;

		bool m_shouldDisplayClosePopup = false;
		std::optional<bool> m_shouldDisplaySafeClose;

		bool m_shouldUpdateDPIScale = true;
		float m_prevDPIScale = 0.0f;
	};
}
