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

			inline Vec2f GetImageSize() const { return m_imageSize; }

			inline bool IsVisible() const { return m_visible; }
			inline bool IsHovered() const { return m_isHovered; }
			inline bool IsFocused() const { return m_isFocused; }
		private:
			friend class MainBar;

			void DrawImage();

			bool m_visible = true;
			bool m_isHovered = false;
			bool m_isFocused = false;

			Vec2i m_imagePosition = 0;
			Vec2i m_imageSize = 0;

			std::weak_ptr<Resource::Texture> m_settingsIcon;
			std::weak_ptr<Resource::Texture> m_menuIcon;
		};
	}
}
