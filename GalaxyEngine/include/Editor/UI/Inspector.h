#pragma once
#include "GalaxyAPI.h"
#include "Editor/UI/EditorWindow.h"
namespace GALAXY {
	namespace Core { class GameObject; }
	namespace Editor::UI
	{
		class File;

		enum class InspectorMode
		{
			None,
			Scene,
			Asset
		};

		class Inspector : public EditorWindow
		{
		public:
			~Inspector() override {}

			void Draw() override;

			void ShowGameObject(Core::GameObject* object);

			void ShowFile(const File* file) const;

			void AddSelected(const Weak<Core::GameObject>& gameObject);
			void SetSelected(const Weak<Core::GameObject>& gameObject);

			void SetFileSelected(List<Shared<File>>* files);
			void UpdateFileSelected();
			void ClearSelectedGameObjects();

			void ClearSelected();

			void RightClickPopup();
			
			const List<Weak<Core::GameObject>>& GetSelectedGameObjects();
		private:
			friend class MainBar;
			InspectorMode m_mode = InspectorMode::None;

			List<Weak<Core::GameObject>> m_selectedGameObject;
			List<Shared<File>>* m_selectedFiles;

			Weak<Component::BaseComponent> m_rightClicked;

		};
	}
}