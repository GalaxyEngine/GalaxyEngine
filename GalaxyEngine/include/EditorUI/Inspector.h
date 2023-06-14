#pragma once
#include "GalaxyAPI.h"
#include "EditorUI/EditorWindow.h"
namespace GALAXY {
	namespace Core { class GameObject; }
	namespace EditorUI
	{
		class Inspector : public EditorWindow
		{
		public:
			~Inspector() {}

			void Draw() override;

			void ShowGameObject(Core::GameObject* object);

			void ShowGameObjects();

			void AddSelected(std::weak_ptr<Core::GameObject> gameObject);

			void SetSelected(std::weak_ptr<Core::GameObject> gameObject);

			void ClearSelected();
			
			std::vector<std::weak_ptr<Core::GameObject>> GetSelected();
		private:
			std::vector<std::weak_ptr<Core::GameObject>> m_selectedGameObject;

		};
	}
}