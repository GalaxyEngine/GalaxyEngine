#pragma once
#include "GalaxyAPI.h"
#include "Editor/UI/EditorWindow.h"
namespace GALAXY {
	namespace Core { class GameObject; }
	namespace Editor::UI
	{
		class Hierarchy : public EditorWindow
		{
		public:
			~Hierarchy() {}

			void Draw() override;

			void DisplayGameObject(Weak<Core::GameObject> weakGO, uint64_t& index, bool display = true);

			void RightClickPopup();

			void SetRename(Core::GameObject* gameObject);

		private:
			friend class MainBar;

			bool m_openRightClick = false;

			bool m_openRename = false;
			Core::GameObject* m_renameObject = nullptr;

			class Inspector* m_inspector = nullptr;

		};
	}
}