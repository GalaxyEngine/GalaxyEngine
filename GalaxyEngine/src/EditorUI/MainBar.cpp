#include "pch.h"
#include "EditorUI/MainBar.h"
#include "EditorUI/EditorUIManager.h"
#include "Core/Application.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#pragma comment(lib, "Comdlg32.lib")
std::string SaveDialog(const char* filter)
{
#ifdef _WIN32
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = Core::Application::GetInstance().GetWindow()->GetWindowWIN32();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
#endif
	return std::string();
}

std::string OpenDialog(const char* filter)
{
#ifdef _WIN32
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = Core::Application::GetInstance().GetWindow()->GetWindowWIN32();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
#endif
	return std::string();
}

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
				if (ImGui::MenuItem("Open Scene"))
				{
					if (auto path = OpenDialog(".scene"); !path.empty())
					{
						//TODO: Load Scene
					}
				}
				if (ImGui::MenuItem("Save Scene"))
				{
					if (auto path = SaveDialog(".scene"); !path.empty())
					{
						//TODO: Save Scene
					}
				}
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

