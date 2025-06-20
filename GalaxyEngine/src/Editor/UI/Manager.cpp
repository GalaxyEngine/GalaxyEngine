#include "pch.h"
#include "Editor/UI/Manager.h"

#include "Editor/UI/SceneWindow.h"
#include "Editor/UI/Hierarchy.h"

#include "Editor/UI/IconManager.h"

#include "Core/Application.h"

#include "Core/SceneHolder.h"
#include "Resource/Scene.h"
#include "Utils/OS.h"

namespace GALAXY {
	Unique<Editor::UI::Manager> Editor::UI::Manager::m_instance;

	Editor::UI::Manager::Manager()
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

	void Editor::UI::Manager::Initialize()
	{
		IconManager::Initialize();
		
		m_context = ImGui::GetCurrentContext();
		m_instance->m_fileExplorer->Initialize();
		m_instance->m_resourceWindow->Initialize();
		m_instance->BindEvents();
	}

	void Editor::UI::Manager::BindEvents()
	{
		std::function<void(const Vec2i&)> bind = std::bind(&Manager::DPIChangeCallback, this, std::placeholders::_1);
		Core::Application::GetInstance().GetWindow()->EOnDPIChange.Bind(bind);
	}

	void Editor::UI::Manager::DrawUI()
	{
		// ImGui::ShowDemoWindow();
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

		if (s_shouldDisplayClosePopup.has_value() && s_shouldDisplayClosePopup.value()) 
		{
			ImGui::OpenPopup("Are you sure ?");
			DisplayClosePopup();
		}
	}

	void Editor::UI::Manager::DisplayClosePopup()
	{
	    static std::vector filters = { Utils::OS::Filter("Galaxy", "galaxy") };
		ImVec2 windowSize = ImGui::GetMainViewport()->Size;
		ImVec2 popupSize(500.0f, 0.0f);
		ImVec2 center = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);
		ImVec2 pos = ImVec2(center.x - popupSize.x * 0.5f, center.y * 0.5f);

		ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(popupSize, ImGuiCond_Appearing);

	    if (ImGui::BeginPopupModal("Are you sure ?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	    {
	        ImGui::TextWrapped("Do you really want to close the scene? Unsaved changes will be lost if you don't save.");

	        ImGui::Dummy(ImVec2(0.0f, 10.0f));
	        ImGui::Separator();
	        ImGui::Dummy(ImVec2(0.0f, 10.0f));

	        float buttonWidth = 120.0f;
	        float spacing = ImGui::GetStyle().ItemSpacing.x;
	        float totalWidth = buttonWidth * 3 + spacing * 2;
	        float cursorX = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;
	        ImGui::SetCursorPosX(cursorX);

	        if (ImGui::Button("Yes", ImVec2(buttonWidth, 0)))
	        {
	            m_onValidatePopup();
	            m_onValidatePopup = nullptr;
	            s_shouldDisplayClosePopup.reset();
	            ImGui::CloseCurrentPopup();
	        }

	        ImGui::SameLine();
	        if (ImGui::Button("Yes and Save", ImVec2(buttonWidth, 0)))
	        {
	            Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
	            if (currentScene->GetFileInfo().Exist())
	            {
	                currentScene->Save(currentScene->GetFileInfo().GetFullPath());
	                m_onValidatePopup();
	                m_onValidatePopup = nullptr;
	                s_shouldDisplayClosePopup.reset();
	            }
	            else
	            {
	                if (const std::string path = Utils::OS::SaveDialog(filters); !path.empty())
	                {
	                    MainBar::SaveScene(path);
	                    m_onValidatePopup();
	                    m_onValidatePopup = nullptr;
	                    s_shouldDisplayClosePopup.reset();
	                }
	            }
	            ImGui::CloseCurrentPopup();
	        }

	        ImGui::SameLine();
	        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
	        {
	            Core::Application::GetInstance().GetWindow()->CancelClose();
	            m_onValidatePopup = nullptr;
	            s_shouldDisplayClosePopup.reset();
	            ImGui::CloseCurrentPopup();
	        }

	        ImGui::EndPopup();
	    }
	}

	void Editor::UI::Manager::SetOnValidatePopupEvent(const std::function<void()>& onValidate)
	{
		m_onValidatePopup = onValidate;
		s_shouldDisplayClosePopup = true;
	}

	bool Editor::UI::Manager::ShouldDisplaySafeClose()
	{
		if (m_instance->s_shouldDisplayClosePopup.has_value())
			return m_instance->s_shouldDisplayClosePopup.value();
		if (Core::Application::IsPlayMode())
			return false;
		
		const auto currentScene = Core::SceneHolder::GetCurrentScene();

		auto objectCount = currentScene->GetObjectList().size();
		
		if (objectCount != 0 && !currentScene->GetFileInfo().Exist()) {

			m_instance->s_shouldDisplayClosePopup.emplace(true);
			return true;
		}

		if (objectCount != 0 && currentScene->WasModified()) {
			m_instance->s_shouldDisplayClosePopup.emplace(true);
			return true;
		}

		return false;
	}

	void Editor::UI::Manager::AddResourceLoading(const Core::UUID& uuid)
	{
		if (std::ranges::find(m_loadingResources, uuid) == m_loadingResources.end())
			m_loadingResources.push_back(uuid);
	}

	void Editor::UI::Manager::RemoveResourceLoading(const Core::UUID& uuid)
	{
		auto v = std::ranges::find(m_loadingResources, uuid);
		if (v != m_loadingResources.end())
			m_loadingResources.erase(v);
	}

	Editor::UI::Manager* Editor::UI::Manager::GetInstance()
	{
		return m_instance.get();
	}

	Editor::UI::Manager* Editor::UI::Manager::CreateInstance()
	{
		m_instance = std::make_unique<Manager>();
		return m_instance.get();
	}

	void Editor::UI::Manager::DrawMainDock()
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

	void Editor::UI::Manager::Release()
	{
		m_instance.reset();
	}

	void Editor::UI::Manager::DPIChangeCallback(const Vec2f& pos)
	{
		m_shouldUpdateDPIScale = true;
	}

	void Editor::UI::Manager::UpdateDPIScale()
	{
		Wrapper::FontManager::AddFonts();
		
		if (!m_shouldUpdateDPIScale)
			return;
		m_shouldUpdateDPIScale = false;

		const float curDPIScale = Core::Application::GetInstance().GetWindow()->GetScreenScale();
		if (m_prevDPIScale == curDPIScale)
			return;
		m_prevDPIScale = curDPIScale;

		if (m_prevDPIScale != 0.f)
			ImGui::GetStyle().ScaleAllSizes(curDPIScale / m_prevDPIScale);
		else
			ImGui::GetStyle().ScaleAllSizes(curDPIScale);

		Wrapper::FontManager::RecreateFonts(curDPIScale);
		
		ImGui_ImplOpenGL3_CreateFontsTexture();
	}
	void* Editor::UI::Manager::GetContext()
	{
		return m_instance->m_context;
	}
}
