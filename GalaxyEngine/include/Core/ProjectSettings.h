#pragma once
#include "GalaxyAPI.h"

#include <filesystem>

#include "Physic/CollisionLayer.h"

namespace GALAXY
{
	namespace Resource
	{
		class Texture;
	}
	
	namespace Core {
		enum class ProjectSettingsTab
		{
			General,
			Collision,
		};

		inline const char* SerializeProjectSettingsTabValue(ProjectSettingsTab e)
		{
			switch (e)
			{
			case ProjectSettingsTab::General: return "General";
			case ProjectSettingsTab::Collision: return "Collision";
			default: return "unknown";
			}
		}
		
		class ProjectSettings
		{
		public:
#ifdef WITH_EDITOR
			void Display();

			void DisplayTab();
			void DisplayGeneralTab();
			void DisplayCollisionTab();

			void DrawTabElement(ProjectSettingsTab tab);
#endif

			void SaveSettings() const;
			void LoadSettings();

			void OnFirstUpdate();
			
			std::filesystem::path GetStartScene() const { return m_startScene; }
			std::filesystem::path GetProjectIconPath() const { return m_projectIcon; }
		private:
			std::filesystem::path m_startScene;
			std::filesystem::path m_projectIcon;
			
			Weak<Resource::Texture> m_projectIconTexture;

			ProjectSettingsTab m_selectedTab = ProjectSettingsTab::General;
			Physic::CollisionLayerManager m_collisionLayerManager;
			
			bool m_firstUpdate = true;

		};
	}
}
