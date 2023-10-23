#pragma once
#include "GalaxyAPI.h"
#include "EditorUI/EditorWindow.h"

namespace GALAXY
{
	namespace Resource
	{
		class Texture;
	}
	namespace EditorUI
	{
		class SceneWindow : public EditorWindow
		{
		public:
			virtual ~SceneWindow();

			void Draw() override;

			void SetResources();

			Vec2f GetMousePosition() const;
			Vec2f GetImageSize() const { return m_imageSize; }

			bool IsVisible() const { return m_visible; }
			bool IsHovered() const { return m_isHovered; }
		private:
			friend class MainBar;

			void DrawImage();

			bool m_visible = true;
			bool m_isHovered = false;

			Vec2i m_imagePosition = 0;
			Vec2i m_imageSize = 0;

			std::weak_ptr<Resource::Texture> m_settingsIcon;
			std::weak_ptr<Resource::Texture> m_menuIcon;
		};
	}
}
