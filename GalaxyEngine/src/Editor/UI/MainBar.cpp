#include "pch.h"
#include "Editor/UI/MainBar.h"
#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"


#include "Utils/OS.h"

namespace GALAXY
{
	void Editor::UI::MainBar::Draw()
	{
		static EditorUIManager* editorInstance = EditorUIManager::GetInstance();
		static EditorSettings& settings = Core::Application::GetInstance().GetEditorSettings();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Scene"))
				{
					if (std::string path = Utils::OS::OpenDialog("All\0*.*\0Galaxy\0*.galaxy\0"); !path.empty())
					{
						if (std::filesystem::path(path).extension() != ".galaxy")
							return;
						OpenScene(path);
					}
				}
				if (ImGui::MenuItem("Save Scene As"))
				{
					if (std::string path = Utils::OS::SaveDialog("Galaxy\0*.galaxy\0"); !path.empty())
					{
						SaveScene(path);
					}
				}
				if (ImGui::MenuItem("Save Scene"))
				{
					std::string path = Core::SceneHolder::GetCurrentScene()->GetFileInfo().GetFullPath().generic_string();
					if (std::filesystem::exists(path)) {
						SaveScene(path);
					}
					else
					{
						if (path = Utils::OS::SaveDialog("Galaxy\0*.galaxy\0"); !path.empty())
						{
							SaveScene(path);
						}
					}
				}
				if (ImGui::MenuItem("Exit"))
				{
					Core::Application::GetInstance().Exit();
				}
				ImGui::EndMenu();
			}
			bool openEditorSettings = false;
			if (ImGui::BeginMenu("Edit"))
			{
				openEditorSettings = ImGui::MenuItem("Editor Settings");
				ImGui::EndMenu();
			}
			if (openEditorSettings)
			{
				ImGui::OpenPopup("EditorSettings");
			}
			settings.Draw();
			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Hierarchy", nullptr, &editorInstance->GetHierarchy()->p_open);
				ImGui::MenuItem("Inspector", nullptr, &editorInstance->GetInspector()->p_open);
				ImGui::MenuItem("SceneWindow", nullptr, &editorInstance->GetSceneWindow()->p_open);
				ImGui::MenuItem("File Explorer", nullptr, &editorInstance->GetFileExplorer()->p_open);
				ImGui::MenuItem("Console", nullptr, &editorInstance->GetConsole()->p_open);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
#ifdef HANDLE_FILE_DIALOG
		// Handle FileDialog
		if (!FileDialog::Initialized())
			return;
		static FileDialog* fileDialog = editorInstance->GetFileDialog();
		if (auto path = fileDialog->GetOutput(); !path.empty())
		{
			if (fileDialog->GetFileDialogType() == FileDialogType::Open) {
				OpenScene(path);
			}
			else
			{
				SaveScene(path);
			}
			fileDialog->SetInitialized(false);
		}
#endif
	}

	void Editor::UI::MainBar::OpenScene(const std::string& path)
	{
		const auto sceneResource = Resource::ResourceManager::ReloadResource<Resource::Scene>(path);

		Core::SceneHolder::GetInstance()->SwitchScene(sceneResource);
	}

	void Editor::UI::MainBar::SaveScene(std::string path)
	{
		if (path.find(".galaxy") == std::string::npos)
			path = path + ".galaxy";

		const Resource::Scene* scene = Core::SceneHolder::GetCurrentScene();
		scene->Save(path);
	}

}

