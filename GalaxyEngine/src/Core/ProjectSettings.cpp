#include "pch.h"
#include "Core/ProjectSettings.h"

#include "Resource/ResourceManager.h"

#include "Utils/OS.h"

namespace GALAXY 
{
	void Core::ProjectSettings::Display()
	{
		static bool firstUpdate = true;
		static ProjectSettings copySettings = *this;
		const std::vector filters = { Utils::OS::Filter("Galaxy", "galaxy") };
		if (ImGui::BeginPopupModal("Project Settings", nullptr))
		{
			if (firstUpdate)
			{
				copySettings = *this;
				firstUpdate = false;
			}
			ImGui::TextUnformatted("Start Scene :");
			ImGui::TreePush("scene");
			if (!m_startScene.empty())
				ImGui::TextUnformatted(m_startScene.generic_string().c_str());
			if (ImGui::Button("Change"))
			{
				m_startScene = Utils::FileInfo::ToRelativePath(Utils::OS::OpenDialog(filters));
			}
			ImGui::TreePop();

			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 100.f * Wrapper::GUI::GetScaleFactor());
			ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 45.f);

			if (ImGui::Button("Cancel"))
			{
				firstUpdate = true;
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

	void Core::ProjectSettings::SaveSettings() const
	{
		auto projectPath = Resource::ResourceManager::GetInstance()->GetProjectPath();

		if (projectPath.empty())
			return;

		CppSer::Serializer serializer(projectPath / "project.settings");

		serializer << CppSer::Pair::BeginMap << "PROJECT SETTINGS";
		serializer << CppSer::Pair::Key << "ProjectPath" << CppSer::Pair::Value << m_startScene;
		serializer << CppSer::Pair::EndMap << "PROJECT SETTINGS";
	}

	void Core::ProjectSettings::LoadSettings()
	{
		auto projectPath = Resource::ResourceManager::GetInstance()->GetProjectPath();

		if (projectPath.empty())
			return;

		CppSer::Parser parser(projectPath / "project.settings");
		m_startScene = parser["ProjectPath"].As<std::string>();
		
	}

}
