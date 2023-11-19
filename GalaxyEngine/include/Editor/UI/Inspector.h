#pragma once
#include "GalaxyAPI.h"
#include "Editor/UI/EditorWindow.h"
namespace GALAXY {
	namespace Core { class GameObject; }
	namespace Editor::UI
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
			
			const List<Weak<Core::GameObject>>& GetSelected();
		private:
			friend class MainBar;

			List<Weak<Core::GameObject>> m_selectedGameObject;

			Weak<Component::BaseComponent> m_rightClicked;

		};
	}
}