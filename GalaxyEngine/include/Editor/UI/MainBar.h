#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Editor::UI
	{
		class MainBar
		{
		public:
			~MainBar() = default;

			static void Draw();

			static void SaveScene(std::string path);

			static void OpenScene(const std::string& path);

		private:

		};
	}
}
