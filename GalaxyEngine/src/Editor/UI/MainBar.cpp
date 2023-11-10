#include "pch.h"
#include "Editor/UI/MainBar.h"
#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

#ifdef _WIN32
#pragma comment(lib, "Comdlg32.lib")
#elif defined(__linux__)
#endif
#define HANDLE_FILE_DIALOG

std::string SaveDialog(const char* filter)
{
#if defined(_WIN32) && !defined(HANDLE_FILE_DIALOG)
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
	ofn.lpstrInitialDir = Resource::ResourceManager::GetInstance()->GetAssetPath().string().c_str();
	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
#elif defined(HANDLE_FILE_DIALOG)
	Editor::UI::FileDialog::OpenFileDialog(Editor::UI::FileDialogType::Save, filter);
#endif
	return "";
}

std::string OpenDialog(const char* filter)
{
#if defined(_WIN32) && !defined(HANDLE_FILE_DIALOG)
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
	ofn.lpstrInitialDir = Resource::ResourceManager::GetInstance()->GetAssetPath().string().c_str();
	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		if (ofn.lpstrFile != NULL)
			return ofn.lpstrFile;
		return "";
	}
#elif defined(HANDLE_FILE_DIALOG)
	Editor::UI::FileDialog::OpenFileDialog(Editor::UI::FileDialogType::Open, filter);
#endif
	return "";
}

namespace GALAXY
{
	Editor::UI::MainBar::MainBar()
	{
	}

	Editor::UI::MainBar::~MainBar()
	{
	}

	void Editor::UI::MainBar::Draw()
	{
		static auto editorInstance = EditorUIManager::GetInstance();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Scene"))
				{
					if (auto path = OpenDialog("All\0*.*\0Galaxy\0*.galaxy\0"); !path.empty())
					{
						OpenScene(path);
					}
				}
				if (ImGui::MenuItem("Save Scene"))
				{
					if (auto path = SaveDialog("Galaxy\0*.galaxy\0"); !path.empty())
					{
						SaveScene(path);
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
				ImGui::MenuItem("Hierarchy", NULL, &editorInstance->GetHierarchy()->p_open);
				ImGui::MenuItem("Inspector", NULL, &editorInstance->GetInspector()->p_open);
				ImGui::MenuItem("SceneWindow", NULL, &editorInstance->GetSceneWindow()->p_open);
				ImGui::MenuItem("File Explorer", NULL, &editorInstance->GetFileExplorer()->p_open);
				ImGui::MenuItem("Console", NULL, &editorInstance->GetConsole()->p_open);
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
		auto sceneResource = Resource::ResourceManager::ReloadResource<Resource::Scene>(path);

		Core::SceneHolder::GetInstance()->SwitchScene(sceneResource);
	}

	void Editor::UI::MainBar::SaveScene(std::string path)
	{
		if (path.find(".galaxy") == std::string::npos)
			path = path + ".galaxy";

		Resource::Scene* scene = Core::SceneHolder::GetInstance()->GetCurrentScene();
		scene->Save(path);
	}

}

