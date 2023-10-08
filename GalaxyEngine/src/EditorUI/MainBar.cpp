#include "pch.h"
#include "EditorUI/MainBar.h"
#include "EditorUI/EditorUIManager.h"
#include "Core/Application.h"

namespace GALAXY 
{
	EditorUI::MainBar::MainBar()
	{
	}

	EditorUI::MainBar::~MainBar()
	{
	}

	void EditorUI::MainBar::Draw()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					Core::Application::GetInstance().Exit();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{
				static auto editorInstance = EditorUIManager::GetInstance();
				ImGui::MenuItem("Hierarchy", NULL, &editorInstance->GetHierarchy()->p_open);
				ImGui::MenuItem("Inspector", NULL, &editorInstance->GetInspector()->p_open);
				ImGui::MenuItem("SceneWindow", NULL, &editorInstance->GetSceneWindow()->p_open);
				ImGui::MenuItem("File Explorer", NULL, &editorInstance->GetFileExplorer()->p_open);
				ImGui::MenuItem("Console", NULL, &editorInstance->GetConsole()->p_open);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

}

