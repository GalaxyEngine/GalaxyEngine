#pragma once
#include "GalaxyAPI.h"
#include "EditorUI/EditorWindow.h"
namespace GALAXY 
{
	namespace EditorUI
	{
		class SceneWindow : public EditorWindow
		{
		public:
			virtual ~SceneWindow();

			void Draw() override;

			bool IsVisible() const { return m_visible; }
		private:
			bool m_visible = true;

		};
	}
}
