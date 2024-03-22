#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Resource
	{
		class Model;
	}
	namespace Editor::UI
	{
		class MainBar
		{
		public:
			~MainBar() = default;

			void Draw();

			static void SaveScene(std::string path);

			static void OpenScene(const std::string& path);

			void AddModelToScene() const;
		private:
			Weak<Resource::Model> m_waitingModel;
		};
	}
}
