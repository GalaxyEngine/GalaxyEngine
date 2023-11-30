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
		if (ImGui::BeginPopupModal("EditorSettings", (bool*)false/*, ImGuiWindowFlags_NoResize*/))
		{
			const float buttonSizeY = 30;
			static Editor::EditorSettings copySettings = *this;
			static float leftSize = 100.f * Wrapper::GUI::GetScaleFactor();
			static float rightSize;
			static Vec2f previousSize = Vec2f(0);
			const Vec2f newSize = ImGui::GetContentRegionAvail();
			if (m_firstUpdate)
			{
				copySettings = *this;
				m_firstUpdate = false;
			}
			if (newSize != previousSize)
			{
				// When resize reset the size of the right size
				rightSize = ImGui::GetContentRegionAvail().x - leftSize;
				previousSize = newSize;
			}

			Wrapper::GUI::Splitter(true, 2, &leftSize, &rightSize, 10, 10);

			ImGui::BeginChild("List", Vec2f(leftSize, ImGui::GetContentRegionAvail().y - buttonSizeY), false);
			AddListElement(EditorSettingsTab::General);
			AddListElement(EditorSettingsTab::ExternalTool);
			AddListElement(EditorSettingsTab::Appearance);
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Panel",  Vec2f(rightSize, ImGui::GetContentRegionAvail().y - buttonSizeY), true);

			DisplayTab(m_selectedTab);

			ImGui::EndChild();

			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 100.f * Wrapper::GUI::GetScaleFactor());

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

	void Editor::EditorSettings::AddListElement(EditorSettingsTab tab)
{
		if (ImGui::Selectable(SerializeEditorSettingsTabValue(tab)))
		{
			m_selectedTab = tab;
		}
	}

	void Editor::EditorSettings::DisplayTab(EditorSettingsTab tab)
	{
		switch (tab)
		{
		case EditorSettingsTab::General:
			break;
		case EditorSettingsTab::ExternalTool:
			DisplayExternalToolTab();
			break;
		case EditorSettingsTab::Appearance:
			DisplayAppearanceTab();
			break;
		default:
			break;
		}
	}

	void Editor::EditorSettings::DisplayExternalToolTab()
	{
		int externalToolID = (int)Core::Application::GetInstance().GetEditorSettings().GetScriptEditorTool();
		if (ImGui::Combo("Script Editor Tool", &externalToolID, Editor::SerializeScriptEditorToolEnum()))
		{
			Core::Application::GetInstance().GetEditorSettings().SetScriptEditorTool((Editor::ScriptEditorTool)externalToolID);
		}
	}

	void Editor::EditorSettings::DisplayAppearanceTab()
	{
		ImGui::ShowStyleEditor();
	}

	void Editor::EditorSettings::SaveSettings()
	{
		Utils::Serializer serializer("Editor.settings");
		serializer << Utils::Pair::BEGIN_MAP << "Editor Settings";
		serializer << Utils::Pair::KEY << "Script Editor Tool" << Utils::Pair::VALUE << (int)GetScriptEditorTool();
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
		m_scriptEditorTool = (Editor::ScriptEditorTool)parser["Script Editor Tool"].As<int>();

	}

}
