#pragma once
#include "GalaxyAPI.h"
#include "EditorWindow.h"
namespace GALAXY
{
	namespace Editor
	{
		namespace UI
		{
			class GameWindow : public EditorWindow
			{
			public:
				void Draw() override;

				void DrawImage();
				bool IsVisible() const { return m_visible; }
				Vec2f GetImageSize() const { return m_imageSize; }
			private:
				friend class MainBar;

				bool m_visible;
				Vec2f m_imageSize = {900, 600};
			};
		}
	}
}
#include "Editor/UI/GameWindow.inl" 
