#pragma once
#include "GalaxyAPI.h"
#include "Core/GameObject.h"

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

			void AddModelToScene() const;

			static void DisplayCreateGameObject(bool& openModelPopup);
			static bool UpdateModelPopup(bool openModelPopup, Core::GameObject* parent = nullptr);
		private:
			Weak<Resource::Model> m_waitingModel;
		};
	}
}
