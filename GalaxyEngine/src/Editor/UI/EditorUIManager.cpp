#include "pch.h"
#include "Editor/UI/EditorUIManager.h"

#include "Editor/UI/SceneWindow.h"
#include "Editor/UI/Hierarchy.h"

#include "Core/Application.h"

#include "Core/SceneHolder.h"
#include "Resource/Scene.h"
#include "Utils/OS.h"

namespace GALAXY {
	Unique<Editor::UI::EditorUIManager> Editor::UI::EditorUIManager::m_instance;

	Editor::UI::EditorUIManager::EditorUIManager()
	{
		m_mainBar = std::make_unique<MainBar>();
		m_sceneWindow = std::make_unique<SceneWindow>();
		m_gameWindow = std::make_unique<GameWindow>();
		m_hierarchy = std::make_unique<Hierarchy>();
		m_inspector = std::make_unique<Inspector>();
		m_console = std::make_unique<Console>();
		m_fileExplorer = std::make_unique<FileExplorer>();
		m_resourceWindow = std::make_unique<ResourceWindow>();
		m_debugWindow = std::make_unique<DebugWindow>();
	}

	void Editor::UI::EditorUIManager::Initialize()
	{
		if (!m_instance)
			m_instance = std::make_unique<EditorUIManager>();
		m_instance->m_mainBar->Initialize();
		m_instance->m_fileExplorer->Initialize();
		m_instance->m_resourceWindow->Initialize();
		m_instance->BindEvents();
	}

	void Editor::UI::EditorUIManager::BindEvents()
	{
		std::function<void(const Vec2i&)> bind = std::bind(&EditorUIManager::MoveWindowCallback, this, std::placeholders::_1);
		Core::Application::GetInstance().GetWindow()->EOnMove.Bind(bind);
	}

	void Editor::UI::EditorUIManager::DrawUI()
	{
		DrawMainDock();
		m_mainBar->Draw();
		m_gameWindow->Draw();
		m_sceneWindow->Draw();
		m_hierarchy->Draw();
		m_inspector->Draw();
		m_fileExplorer->Draw();
		m_console->Draw();
		m_resourceWindow->Draw();
		m_debugWindow->Draw();

		if (m_shouldDisplayClosePopup) 
		{
			ImGui::OpenPopup("Are you sure ?");
			DisplayClosePopup();
		}
	}

	void Editor::UI::EditorUIManager::DisplayClosePopup()
	{
		static std::vector filters = { Utils::OS::Filter("Galaxy", "galaxy") };
		if (ImGui::BeginPopupModal("Are you sure ?")) {
			if (ImGui::Button("Yes")) {

				Core::Application::GetInstance().GetWindow()->ForceClose();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Yes and save")) {
				Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
				if (currentScene->GetFileInfo().Exist())
				{
					currentScene->Save(currentScene->GetFileInfo().GetFullPath());
					Core::Application::GetInstance().GetWindow()->ForceClose();
				}
				else
				{
					if (const std::string path = Utils::OS::SaveDialog(filters); !path.empty())
					{
						MainBar::SaveScene(path);
						Core::Application::GetInstance().GetWindow()->ForceClose();
					}
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				Core::Application::GetInstance().GetWindow()->CancelClose();
				m_shouldDisplaySafeClose.reset();
				m_shouldDisplayClosePopup = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	bool Editor::UI::EditorUIManager::ShouldDisplaySafeClose()
	{
		if (m_shouldDisplaySafeClose.has_value())
			return m_shouldDisplaySafeClose.value();

		const auto currentScene = Core::SceneHolder::GetCurrentScene();
		if (!currentScene->GetFileInfo().Exist()) {

			m_shouldDisplaySafeClose.emplace(true);
			return true;
		}

		if (currentScene->WasModified()) {
			m_shouldDisplaySafeClose.emplace(true);
			return true;
		}

		return false;
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

	void Editor::UI::EditorUIManager::MoveWindowCallback(const Vec2i& pos)
	{
		m_shouldUpdateDPIScale = true;
	}

	void Editor::UI::EditorUIManager::UpdateDPIScale()
	{
		if (!m_shouldUpdateDPIScale)
			return;
		m_shouldUpdateDPIScale = false;

		float curDPIScale = Core::Application::GetInstance().GetWindow()->GetScreenScale();
		if (m_prevDPIScale == curDPIScale)
			return;
		m_prevDPIScale = curDPIScale;

		if (m_prevDPIScale != 0)
			ImGui::GetStyle().ScaleAllSizes(curDPIScale / m_prevDPIScale);
		else
			ImGui::GetStyle().ScaleAllSizes(curDPIScale);

		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		io.Fonts->AddFontFromFileTTF(ENGINE_RESOURCE_FOLDER_NAME"/fonts/Calibri.ttf", 13 * curDPIScale);
		ImGui_ImplOpenGL3_CreateFontsTexture();
	}

}
