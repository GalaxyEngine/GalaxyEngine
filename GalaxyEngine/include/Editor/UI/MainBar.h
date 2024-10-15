#pragma once
#include "GalaxyAPI.h"
#include "Utils/Type.h"
#include <string>

namespace GALAXY 
{
	namespace Core
	{
		class GameObject;
	}
	
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

			static void DisplayCreateGameObject(bool& openModelPopup, Core::GameObject* parent = nullptr);
			static bool UpdateModelPopup(bool openModelPopup, Core::GameObject* parent = nullptr);
		private:
			Weak<Resource::Model> m_waitingModel;

		};
	}
}
