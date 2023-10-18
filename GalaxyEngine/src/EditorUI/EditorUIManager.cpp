#include "pch.h"
#include "EditorUI/EditorUIManager.h"

#include "EditorUI/SceneWindow.h"
#include "EditorUI/Hierarchy.h"

namespace GALAXY {
	std::unique_ptr<EditorUI::EditorUIManager> EditorUI::EditorUIManager::m_instance;

	EditorUI::EditorUIManager::EditorUIManager()
	{
		m_mainBar = std::make_unique<MainBar>();
		m_sceneWindow = std::make_unique<SceneWindow>();
		m_hierarchy = std::make_unique<Hierarchy>();
		m_inspector = std::make_unique<Inspector>();
		m_console = std::make_unique<Console>();
		m_fileExplorer = std::make_unique<FileExplorer>();
	}

	void EditorUI::EditorUIManager::Initialize()
	{
		if (!m_instance)
			m_instance = std::make_unique<EditorUIManager>();
		m_instance->m_fileExplorer->Initialize();
	}

	void EditorUI::EditorUIManager::DrawUI()
	{
		DrawMainDock();
		m_mainBar->Draw();
		m_sceneWindow->Draw();
		m_hierarchy->Draw();
		m_inspector->Draw();
		m_console->Draw();
		m_fileExplorer->Draw();
	}

	EditorUI::EditorUIManager* GALAXY::EditorUI::EditorUIManager::GetInstance()
	{
		return m_instance.get();
	}

	void EditorUI::EditorUIManager::DrawMainDock()
	{
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;
		ImGui::GetWindowDockID();

		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red
		ImGui::Begin("DockSpace", (bool*)true, window_flags);
		ImGui::PopStyleColor();
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();
	}

	void EditorUI::EditorUIManager::Release()
	{
		m_instance.reset();
	}

}