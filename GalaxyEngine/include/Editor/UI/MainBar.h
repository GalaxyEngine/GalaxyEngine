#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Editor::UI
	{
		class MainBar
		{
		public:
			MainBar();
			~MainBar();

			void Draw();

			void SaveScene(std::string path);

			void OpenScene(const std::string& path);

		private:

		};
	}
}
