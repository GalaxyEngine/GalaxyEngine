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

			void DisplayGameObject(std::weak_ptr<Core::GameObject> weakGO, uint32_t index = 0);

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