#pragma once
#include "GalaxyAPI.h"

#include <filesystem>
namespace GALAXY
{
	namespace Resource
	{
		class Texture;
	}
	
	namespace Core {
		class ProjectSettings
		{
		public:
			void Display();

			void SaveSettings() const;
			void LoadSettings();
			
			std::filesystem::path GetStartScene() const { return m_startScene; }
			std::filesystem::path GetProjectIconPath() const { return m_projectIcon; }
		private:
			std::filesystem::path m_startScene;
			
			std::filesystem::path m_projectIcon;
			Weak<Resource::Texture> m_projectIconTexture;

		};
	}
}