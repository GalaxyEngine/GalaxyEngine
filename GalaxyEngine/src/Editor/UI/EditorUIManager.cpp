#include "pch.h"
#include "Editor/UI/EditorUIManager.h"

#include "Editor/UI/SceneWindow.h"
#include "Editor/UI/Hierarchy.h"

#include "Core/Application.h"

namespace GALAXY {
	Unique<Editor::UI::EditorUIManager> Editor::UI::EditorUIManager::m_instance;

	Editor::UI::EditorUIManager::EditorUIManager()
	{
		m_mainBar = std::make_unique<MainBar>();
		m_sceneWindow = std::make_unique<SceneWindow>();
		m_hierarchy = std::make_unique<Hierarchy>();
		m_inspector = std::make_unique<Inspector>();
		m_console = std::make_unique<Console>();
		m_fileExplorer = std::make_unique<FileExplorer>();
		m_fileDialog = std::make_unique<FileDialog>();
	}

	void Editor::UI::EditorUIManager::Initialize()
	{
		if (!m_instance)
			m_instance = std::make_unique<EditorUIManager>();
		m_instance->m_fileExplorer->Initialize();
	}

	void Editor::UI::EditorUIManager::DrawUI()
	{
		DrawMainDock();
		m_fileDialog->Draw();
		m_mainBar->Draw();
		m_sceneWindow->Draw();
		m_hierarchy->Draw();
		m_inspector->Draw();
		m_fileExplorer->Draw();
		m_console->Draw();
	}

	Editor::UI::EditorUIManager* Editor::UI::EditorUIManager::GetInstance()
	{
		return m_instance.get();
	}

	void Editor::UI::EditorUIManager::DrawMainDock()
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
		ImGui::Begin("DockSpace", nullptr, window_flags);
		ImGui::PopStyleColor();
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		const ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();
	}

	void Editor::UI::EditorUIManager::Release()
	{
		m_instance.reset();
	}

}