#include "pch.h"
#include "Editor/UI/MainBar.h"
#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Resource/Model.h"

#include "Utils/OS.h"

namespace GALAXY
{

	void Editor::UI::MainBar::Initialize()
	{
	}

	void Editor::UI::MainBar::Draw()
	{
		const std::vector filters = { Utils::OS::Filter("Galaxy", "galaxy") };
		EditorUIManager* editorInstance = EditorUIManager::GetInstance();
		EditorSettings& editorSettings = Core::Application::GetInstance().GetEditorSettings();
		Core::ProjectSettings& projectSettings = Core::Application::GetInstance().GetProjectSettings();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Scene"))
				{
					if (const std::string path = Utils::OS::OpenDialog(filters); !path.empty())
					{
						if (std::filesystem::path(path).extension() != ".galaxy")
							return;
						OpenScene(path);
					}
				}
				if (ImGui::MenuItem("Save Scene As"))
				{
					if (const std::string path = Utils::OS::SaveDialog(filters); !path.empty())
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
						if (path = Utils::OS::SaveDialog(filters); !path.empty())
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
			bool openProjectSettings = false;
			if (ImGui::BeginMenu("Edit"))
			{
				openEditorSettings = ImGui::MenuItem("Editor Settings");
				openProjectSettings = ImGui::MenuItem("Project Settings");
				ImGui::EndMenu();
			}
			if (openEditorSettings)
			{
				ImGui::OpenPopup("Editor Settings");
			}
			editorSettings.Display();
			if (openProjectSettings)
			{
				ImGui::OpenPopup("Project Settings");
			}
			projectSettings.Display();

			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Hierarchy", nullptr, &editorInstance->GetHierarchy()->p_open);
				ImGui::MenuItem("Inspector", nullptr, &editorInstance->GetInspector()->p_open);
				ImGui::MenuItem("SceneWindow", nullptr, &editorInstance->GetSceneWindow()->p_open);
				ImGui::MenuItem("GameWindow", nullptr, &editorInstance->GetGameWindow()->p_open);
				ImGui::MenuItem("File Explorer", nullptr, &editorInstance->GetFileExplorer()->p_open);
				ImGui::MenuItem("Console", nullptr, &editorInstance->GetConsole()->p_open);
				ImGui::MenuItem("Resources", nullptr, &editorInstance->GetResourceWindow()->p_open);
				ImGui::EndMenu();
			}
			bool openCreateWithModel = false;
			if (ImGui::BeginMenu("GameObject"))
			{
				if (ImGui::MenuItem("Create With Model"))
				{
					openCreateWithModel = true;
				}
				ImGui::EndMenu();
			}
			if (openCreateWithModel)
			{
				ImGui::OpenPopup("Create With Model");
			}
			Weak<Resource::Model> model;
			if (Resource::ResourceManager::GetInstance()->ResourcePopup("Create With Model", model))
			{
				if (auto modelShared = model.lock())
				{
					m_waitingModel = modelShared;
					if (!modelShared->IsLoaded()) {
						auto bind = [this] { AddModelToScene(); };
						modelShared->OnLoad.Bind(bind);
						return;
					}
					AddModelToScene();
				}
			}
			ImGui::EndMainMenuBar();
		}
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

	void Editor::UI::MainBar::AddModelToScene()
	{
		if (!m_waitingModel.lock())
			return;
		const auto object = m_waitingModel.lock()->ToGameObject();
		Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
		currentScene->AddObject(object);
		currentScene->GetRootGameObject().lock()->AddChild(object);
	}

}

