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

			void AddSelected(Weak<Core::GameObject> gameObject);

			void SetSelected(Weak<Core::GameObject> gameObject);

			void ClearSelected();

			void RightClickPopup();
			
			List<Weak<Core::GameObject>> GetSelected();
		private:
			friend class MainBar;

			List<Weak<Core::GameObject>> m_selectedGameObject;

			Weak<Component::BaseComponent> m_rightClicked;

		};
	}
}