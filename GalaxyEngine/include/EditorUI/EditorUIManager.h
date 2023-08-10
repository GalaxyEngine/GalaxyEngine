#pragma once
#include "GalaxyAPI.h"
#include "EditorUI/Hierarchy.h"
#include "EditorUI/Inspector.h"
#include "EditorUI/SceneWindow.h"

namespace GALAXY::EditorUI {
	class EditorUIManager
	{
	public:
		~EditorUIManager() {}

		static EditorUIManager* GetInstance();

		void Initalize();

		void DrawUI();

		void DrawMainDock();

		Hierarchy* GetHierarchy() const { return m_hierarchy.get(); }
		Inspector* GetInspector() const { return m_inspector.get(); }
		SceneWindow* GetSceneWindow() const { return m_sceneWindow.get(); }
	private:
		static std::unique_ptr<EditorUIManager> m_instance;
		std::unique_ptr<Hierarchy> m_hierarchy;
		std::unique_ptr<Inspector> m_inspector;
		std::unique_ptr<SceneWindow> m_sceneWindow;
	};
}
