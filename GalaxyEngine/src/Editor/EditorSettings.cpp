#include "pch.h"
#include "Editor/EditorSettings.h"

#include "Core/Application.h"

#include "Utils/Parser.h"
#include "Utils/FileInfo.h"

namespace GALAXY
{

	Editor::EditorSettings& Editor::EditorSettings::GetInstance()
	{
		return Core::Application::GetInstance().GetEditorSettings();
	}

	void Editor::EditorSettings::Draw()
	{
		//TODO : Improve
		if (ImGui::BeginPopupModal("EditorSettings"))
		{
			static Editor::EditorSettings copySettings = *this;
			if (m_firstUpdate)
			{
				copySettings = *this;
				m_firstUpdate = false;
			}
			Vec2f buttonSize = Vec2f(0, 30);
			ImGui::BeginChild("Panel", ImGui::GetContentRegionAvail() - buttonSize, true);
			int externalToolID = (int)Core::Application::GetInstance().GetEditorSettings().GetExternalTool();
			if (ImGui::Combo("External Tool", &externalToolID, Editor::SerializeExternalToolEnum()))
			{
				Core::Application::GetInstance().GetEditorSettings().SetExternalTool((Editor::ExternalTool)externalToolID);
			}
			ImGui::EndChild();

			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 100.f);

			if (ImGui::Button("Cancel"))
			{
				m_firstUpdate = true;
				*this = copySettings;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Save"))
			{
				SaveSettings();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void Editor::EditorSettings::SaveSettings()
	{
		Utils::Serializer serializer("Editor.settings");
		serializer << Utils::Pair::BEGIN_MAP << "Editor Settings";
		serializer << Utils::Pair::KEY << "ExternalTool" << Utils::Pair::VALUE << (int)GetExternalTool();
		serializer << Utils::Pair::END_MAP << "Editor Settings";
	}

	void Editor::EditorSettings::LoadSettings()
	{
		Utils::Parser parser(Path("Editor.settings"));
		if (!parser.IsFileOpen())
		{
			PrintError("Can't open Editor.settings");
			return;
		}
		m_externalTool = (Editor::ExternalTool)parser["ExternalTool"].As<int>();

	}

}
