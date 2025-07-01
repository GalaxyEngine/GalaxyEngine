#include "pch.h"
#include "Core/ProjectSettings.h"

#include "Physic/CollisionLayer.h"

#include "Resource/ResourceManager.h"

#include "Utils/OS.h"

#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
#ifdef WITH_EDITOR
	void Core::ProjectSettings::Display()
	{
		static ProjectSettings copySettings = *this;
		if (ImGui::BeginPopupModal("Project Settings", nullptr))
		{
            constexpr float buttonSizeY = 30;
			static float leftSize = 150.f;
			static float rightSize;
			static Vec2f previousSize = Vec2f(0);
			const Vec2f newSize = ImGui::GetContentRegionAvail();
			if (m_firstUpdate)
			{
				OnFirstUpdate();
				copySettings = *this;
				m_firstUpdate = false;
			}
			if (newSize != previousSize)
			{
				// When resize reset the size of the right size
				rightSize = ImGui::GetContentRegionAvail().x - leftSize;
				previousSize = newSize;
			}

			Wrapper::UI::Splitter(true, 2, &leftSize, &rightSize, 10, 10);
			
			ImGui::BeginChild("List", Vec2f(leftSize, ImGui::GetContentRegionAvail().y - buttonSizeY), false);
			DrawTabElement(ProjectSettingsTab::General);
			DrawTabElement(ProjectSettingsTab::Collision);
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Panel", Vec2f(rightSize, ImGui::GetContentRegionAvail().y - buttonSizeY), true);

			DisplayTab();

			ImGui::EndChild();

			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 100.f * Wrapper::UI::GetScaleFactor());

			if (ImGui::Button("Cancel"))
			{
				m_firstUpdate = true;
				*this = copySettings;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Save"))
			{
				m_firstUpdate = true;
				Physic::CollisionLayerManager::GetInstance() = m_collisionLayerManager;
				SaveSettings();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void Core::ProjectSettings::DisplayTab()
	{
		switch (m_selectedTab) {
		case ProjectSettingsTab::General:
			DisplayGeneralTab();
			break;
		case ProjectSettingsTab::Collision:
			DisplayCollisionTab();
			break;
		}
	}

	void Core::ProjectSettings::DisplayGeneralTab()
	{
		const std::vector filters = { Utils::OS::Filter("Galaxy", "galaxy") };
		ImGui::TextUnformatted("Start Scene :");
		ImGui::TreePush("scene");
		if (!m_startScene.empty())
			ImGui::TextUnformatted(m_startScene.generic_string().c_str());
		if (ImGui::Button("Change"))
		{
			m_startScene = Utils::FileInfo::ToRelativePath(Utils::OS::OpenDialog(filters));
		}
		ImGui::TreePop();

		if (ImGui::Button("Set Project Icon"))
		{
			const std::vector filters = { Utils::OS::Filter("Image", "png,jpg,jpeg") };
			m_projectIcon = Utils::FileInfo::ToRelativePath(Utils::OS::OpenDialog(filters));
			m_projectIconTexture = Resource::ResourceManager::GetOrLoad<Resource::Texture>(m_projectIcon);
		}

		Wrapper::UI::TextureImage(m_projectIconTexture.lock().get(), Vec2f(64.f, 64.f), Vec2i(0, 0), Vec2i(1, 1));

	}

	void Core::ProjectSettings::DisplayCollisionTab()
	{
		Physic::CollisionMatrix& layers = m_collisionLayerManager.GetCollisionMatrix();
		Wrapper::PhysicsWrapper* physicsWrapper = Wrapper::PhysicsWrapper::GetInstance();
		auto threshold = physicsWrapper->GetSleepThreshold();

		if (ImGui::InputFloat("Sleep Threshold", &threshold, 0.001f, 0.01f, "%.6f"))
		{
			physicsWrapper->SetSleepThreshold(threshold);
		}
		ImGui::TextUnformatted("Collision Matrix");
		ImGui::Separator();
		const int numLayers = static_cast<int>(layers.size());
		if (numLayers == 0)
			return;

		// Create a table with one extra column for the row headers.
		if (ImGui::BeginTable("CollisionMatrix", numLayers + 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableNextRow();
			// Empty header cell for the first column.
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(" ");
			// Column headers: layer names.
			for (int col = 0; col < numLayers; col++)
			{
				ImGui::TableSetColumnIndex(col + 1);
				ImGui::Text("%s", Physic::Layer::GetLayerName(col).c_str());
			}

			for (int row = 0; row < numLayers; row++)
			{
				ImGui::TableNextRow();
				// Row header: display layer name.
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", Physic::Layer::GetLayerName(row).c_str());

				for (int col = 0; col < numLayers; col++)
				{
					ImGui::TableSetColumnIndex(col + 1);

					// Copy the current value from the matrix.
					bool cellValue = layers[row][col];
					std::string cellLabel = "##" + std::to_string(row) + "_" + std::to_string(col);

					// Render the checkbox.
					if (ImGui::Checkbox(cellLabel.c_str(), &cellValue))
					{
						layers[row][col] = cellValue;
						layers[col][row] = cellValue;
					}
				}
			}

			ImGui::EndTable();
		}
		if (ImGui::Button("Enable All"))
		{
			for (int i = 0; i < numLayers; i++)
			{
				for (int j = 0; j < numLayers; j++)
				{
					layers[i][j] = true;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Disable All"))
		{
			for (int i = 0; i < numLayers; i++)
			{
				for (int j = 0; j < numLayers; j++)
				{
					layers[i][j] = false;
				}
			}
		}

		for (int i = 0; i < numLayers; i++)
		{
			ImGui::PushID(i);
			std::string name = m_collisionLayerManager.GetLayerName(i);
			if (Wrapper::UI::InputText("##layer", &name, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				m_collisionLayerManager.ChangeLayerName(i, name);
			}
			ImGui::SameLine();
			if (ImGui::Button("-"))
			{
				m_collisionLayerManager.RemoveLayer(i);
				break;
			}
			ImGui::PopID();
		}
		if (ImGui::Button("Add Layer"))
		{
			std::string name = "Layer ";
			int index = 0;
			do {
				index++;
			}
			while (m_collisionLayerManager.ContainsLayer(name + std::to_string(index)));
			m_collisionLayerManager.AddLayer(name + std::to_string(index));
		}
	}

	void Core::ProjectSettings::DrawTabElement(ProjectSettingsTab tab)
	{
		if (ImGui::Selectable(SerializeProjectSettingsTabValue(tab)))
		{
			m_selectedTab = tab;
		}
	}
#endif

	void Core::ProjectSettings::SaveSettings() const
	{
		Path projectPath = Resource::ResourceManager::GetProjectPath();

		if (projectPath.empty())
			return;

		CppSer::Serializer serializer(projectPath / "project.settings");

		serializer << CppSer::Pair::BeginMap << "PROJECT SETTINGS";
		serializer << CppSer::Pair::Key << "Start Scene" << CppSer::Pair::Value << m_startScene;
		serializer << CppSer::Pair::Key << "Project Icon" << CppSer::Pair::Value << m_projectIcon;

		auto physicWrapper = Wrapper::PhysicsWrapper::GetInstance();
		serializer << CppSer::Pair::Key << "Sleep Threshold" << CppSer::Pair::Value << physicWrapper->GetSleepThreshold();  

		const Physic::CollisionMatrix& layers = m_collisionLayerManager.GetCollisionMatrix();
		const int numLayers = static_cast<int>(layers.size());
		serializer << CppSer::Pair::Key << "Num Layers" << CppSer::Pair::Value << numLayers;
		for (int i = 0; i < numLayers; i++)
		{
			serializer << CppSer::Pair::Key << "Layer " + std::to_string(i) << CppSer::Pair::Value << m_collisionLayerManager.GetLayerName(i);
		}

		for (int i = 0; i < numLayers; i++)
		{
			for (int j = 0; j < numLayers; j++)
			{
				serializer << CppSer::Pair::Key << "Collision " + std::to_string(i) + "_" + std::to_string(j) << CppSer::Pair::Value << layers[i][j];
			}
		}
		
		serializer << CppSer::Pair::EndMap << "PROJECT SETTINGS";
	}

	void Core::ProjectSettings::LoadSettings()
	{
		auto projectPath = Resource::ResourceManager::GetProjectPath();
		if (projectPath.empty())
			return;

		CppSer::Parser parser(projectPath / "project.settings");
		if (!parser.IsFileOpen())
			return;
		m_startScene = parser["Start Scene"].As<std::string>();
		m_projectIcon = parser["Project Icon"].As<std::string>();

		if (!m_projectIcon.empty())
		{
			m_projectIconTexture = Resource::ResourceManager::GetOrLoad<Resource::Texture>(m_projectIcon);
		}
		auto physicWrapper = Wrapper::PhysicsWrapper::GetInstance();
		if (parser.HasKey("Sleep Threshold"))
		{
			float sleepThreshold = parser["Sleep Threshold"].As<float>();
			physicWrapper->SetSleepThreshold(sleepThreshold);
		}

		const int numLayers = parser["Num Layers"].As<int>();
		for (int i = 0; i < numLayers; i++)
		{
			std::string layerName = parser["Layer " + std::to_string(i)].As<std::string>();
			if (!m_collisionLayerManager.ContainsLayer(layerName))
				m_collisionLayerManager.AddLayer(layerName);
		}

		for (int i = 0; i < numLayers; i++)
		{
			for (int j = 0; j < numLayers; j++)
			{
				bool canCollide = parser["Collision " + std::to_string(i) + "_" + std::to_string(j)].As<bool>();
				m_collisionLayerManager.SetCollision(i, j, canCollide);
			}
		}

		Physic::CollisionLayerManager::GetInstance() = m_collisionLayerManager;
	}

	void Core::ProjectSettings::OnFirstUpdate()
	{
		m_collisionLayerManager = Wrapper::PhysicsWrapper::GetInstance()->GetCollisionLayerManager();
	}
}
