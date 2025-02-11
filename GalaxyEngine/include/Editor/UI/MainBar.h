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
	using WaitingModelMap = std::map<int, std::pair<Shared<Resource::Model>, Core::GameObject*>>;
	namespace Editor::UI
	{
		class MainBar
		{
		public:
			~MainBar() = default;

			void Draw();

			static void SaveScene(std::string path);

			static void DisplayCreateGameObject(bool& openModelPopup, Core::GameObject* parent = nullptr);
			static bool UpdateModelPopup(bool openModelPopup, Core::GameObject* parent = nullptr);
		private:
			static WaitingModelMap m_waitingModels;
		};
	}
}
