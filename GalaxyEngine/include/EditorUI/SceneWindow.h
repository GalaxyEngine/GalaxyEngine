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

			bool IsVisible() const { return m_visible; }
			bool IsHovered() const { return m_isHovered; }
		private:
			friend class MainBar;

			void DrawImage();

			bool m_visible = true;
			bool m_isHovered = false;

			Vec2f m_windowPosition = 0;

			std::weak_ptr<Resource::Texture> m_settingsIcon;
			std::weak_ptr<Resource::Texture> m_menuIcon;
		};
	}
}
