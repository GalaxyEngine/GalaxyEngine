#include "pch.h"
#include "Editor/EditorSettings.h"

#include "Core/Application.h"

namespace GALAXY 
{

	Editor::EditorSettings& Editor::EditorSettings::GetInstance()
	{
		return Core::Application::GetInstance().GetEditorSettings();
	}

	void Editor::EditorSettings::Draw()
	{
		if (ImGui::BeginPopup("EditorSettings"))
		{
			//TODO : Improve
			int externalToolID = (int)Core::Application::GetInstance().GetEditorSettings().GetExternalTool();
			if (ImGui::Combo("External Tool", &externalToolID, Editor::SerializeExternalToolEnum()))
			{
				Core::Application::GetInstance().GetEditorSettings().SetExternalTool((Editor::ExternalTool)externalToolID);
			}
			ImGui::EndPopup();
		}
	}

}
