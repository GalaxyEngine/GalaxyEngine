#pragma once
#include "GalaxyAPI.h"
#include <galaxymath/Maths.h>
#include "Editor/UI/EditorWindow.h"

namespace GALAXY
{
	namespace Core
	{
		class GameObject;
	}

	namespace Resource
	{
		class Model;
		class Texture;
	}
	namespace Editor::UI
	{
		class SceneWindow : public EditorWindow
		{
		public:
			virtual ~SceneWindow();

			void Draw() override;

			void SetResources();

			Vec2f GetMousePosition() const;

			inline Vec2f GetImageSize() const { return m_imageSize; }

			inline bool IsVisible() const { return m_visible; }
			inline bool IsHovered() const { return m_isHovered; }
			inline bool IsFocused() const { return m_isFocused; }

#ifdef WITH_EDITOR
			void UpdateDragModel();
			void OnModelLoaded(Weak<Resource::Model> model);
#endif
		private:
			friend class MainBar;

			void DrawImage();

			bool m_visible = true;
			bool m_isHovered = false;
			bool m_isFocused = false;

			Vec2i m_imagePosition = Vec2i(0);
			Vec2i m_imageSize = Vec2i(0);

			std::weak_ptr<Resource::Texture> m_settingsIcon;
			std::weak_ptr<Resource::Texture> m_menuIcon;
			
#ifdef WITH_EDITOR
			Weak<Resource::Model> m_dragModel;
			Shared<Core::GameObject> m_dragModelObject = nullptr;
#endif
		};
	}
}
