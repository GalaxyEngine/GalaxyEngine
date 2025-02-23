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
				Vec2f GetImageSize() const { return m_imageSize; }

				Vec2i GetMousePosition() const;

			private:
				friend class MainBar;

				Vec2f m_imagePosition;
				Vec2f m_imageSize = {900, 600};
			};
		}
	}
}
#include "Editor/UI/GameWindow.inl" 
