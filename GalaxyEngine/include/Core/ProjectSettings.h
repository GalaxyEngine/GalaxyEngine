#pragma once
#include "GalaxyAPI.h"

#include <filesystem>
namespace GALAXY
{
	namespace Core {
		class ProjectSettings
		{
		public:
			void Display();

			void SaveSettings() const;
			void LoadSettings();
			std::filesystem::path GetStartScene() const { return m_startScene; }
		private:
			std::filesystem::path m_startScene;

		};
	}
}