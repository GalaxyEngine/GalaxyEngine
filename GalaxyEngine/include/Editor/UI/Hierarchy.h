#pragma once
#include "GalaxyAPI.h"
#include "Editor/UI/EditorWindow.h"
#include "Utils/Type.h"

namespace GALAXY {
	namespace Core { class GameObject; }
	namespace Editor::UI
	{
		class Hierarchy : public EditorWindow
		{
		public:
			~Hierarchy() override {}

			void Draw() override;

			// Display the GameObject and its children in the Hierarchy
			// @param index: index of the object in the hierarchy
			// @param display: if the object should be displayed
			void DisplayGameObject(const Weak<Core::GameObject>& weakGO, uint64_t& index, bool display = true);

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