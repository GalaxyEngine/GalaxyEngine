#include "pch.h"

#include <numeric>

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
		if (ImGui::BeginPopupModal("EditorSettings", nullptr/*, ImGuiWindowFlags_NoResize*/))
		{
			constexpr float buttonSizeY = 30;
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
			AddListElement(EditorSettingsTab::Benchmark);
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Panel", Vec2f(rightSize, ImGui::GetContentRegionAvail().y - buttonSizeY), true);

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

	void Editor::EditorSettings::AddListElement(const EditorSettingsTab tab)
	{
		if (ImGui::Selectable(SerializeEditorSettingsTabValue(tab)))
		{
			m_selectedTab = tab;
		}
	}

	void Editor::EditorSettings::DisplayTab(const EditorSettingsTab tab)
	{
		switch (tab)
		{
		case EditorSettingsTab::General:
			DisplayGeneralTab();
			break;
		case EditorSettingsTab::ExternalTool:
			DisplayExternalToolTab();
			break;
		case EditorSettingsTab::Appearance:
			DisplayAppearanceTab();
			break;
		case EditorSettingsTab::Benchmark:
			DisplayBenchmarkTab();
			break;
		default:
			break;
		}
	}

	void Editor::EditorSettings::DisplayGeneralTab()
	{
	}

	void Editor::EditorSettings::DisplayExternalToolTab()
	{
		int externalToolID = static_cast<int>(Core::Application::GetInstance().GetEditorSettings().GetScriptEditorTool());
		if (ImGui::Combo("Script Editor Tool", &externalToolID, SerializeScriptEditorToolEnum()))
		{
			Core::Application::GetInstance().GetEditorSettings().SetScriptEditorTool(static_cast<ScriptEditorTool>(externalToolID));
		}
	}

	void Editor::EditorSettings::DisplayAppearanceTab()
	{
		ImGui::ShowStyleEditor();
	}

	void Editor::EditorSettings::DisplayBenchmarkTab()
	{
		static float benchmarkTimeSeconds = 60;
		static float benchmarkUpdateFrequency = 0.5f;

		static Benchmark& benchmark = Core::Application::GetInstance().GetBenchmark();

		ImGui::BeginDisabled(benchmark.IsRunning());

		ImGui::InputFloat("Benchmark Time (seconds)", &benchmarkTimeSeconds, 0.1f, 0, "%.1f");
		ImGui::InputFloat("Update Frequency (seconds)", &benchmarkUpdateFrequency, 0.1f, 0, "%.1f");
		if (ImGui::Button("Start Benchmark"))
		{
			auto benchmarkSettings = BenchmarkSettings();
			benchmarkSettings.time = benchmarkTimeSeconds;
			benchmarkSettings.updateFrequency = benchmarkUpdateFrequency;
			benchmark.SetBenchmarkSettings(benchmarkSettings);
			benchmark.StartBenchmark();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		if (benchmark.IsRunning())
		{
			ImGui::Text("Average FPS %f", benchmark.GetAverageFPS());
		}
	}

	void Editor::EditorSettings::SaveSettings() const
	{
		Utils::Serializer serializer("Editor.settings");
		serializer << Pair::BEGIN_MAP << "Editor Settings";
		serializer << Pair::KEY << "Script Editor Tool" << Pair::VALUE << static_cast<int>(GetScriptEditorTool());
		serializer << Pair::END_MAP << "Editor Settings";
	}

	void Editor::EditorSettings::LoadSettings()
	{
		Utils::Parser parser(Path("Editor.settings"));
		if (!parser.IsFileOpen())
		{
			PrintError("Can't open Editor.settings");
			return;
		}
		m_scriptEditorTool = static_cast<Editor::ScriptEditorTool>(parser["Script Editor Tool"].As<int>());

	}

}
