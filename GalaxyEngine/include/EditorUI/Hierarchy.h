#pragma once
#include "GalaxyAPI.h"
#include "EditorUI/EditorWindow.h"
namespace GALAXY {
	namespace Core { class GameObject; }
	namespace EditorUI
	{
		class Hierarchy : public EditorWindow
		{
		public:
			~Hierarchy() {}

			void Draw() override;

			void DisplayGameObject(Core::GameObject* gameobject, uint32_t index = 0);
		private:

		};
	}
}