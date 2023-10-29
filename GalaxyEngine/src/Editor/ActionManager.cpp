#include "pch.h"
#include "Editor/ActionManager.h"
namespace GALAXY 
{

	void Editor::ActionManager::Update()
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z))
		{
			Undo();
		}
		else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Y))
		{
			Redo();
		}
	}

}
