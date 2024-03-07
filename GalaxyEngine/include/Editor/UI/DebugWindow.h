#pragma once
#include "GalaxyAPI.h"

#include "Editor/UI/EditorWindow.h"

namespace GALAXY 
{
	namespace Editor::UI
	{
		class DebugWindow : public EditorWindow
		{
		public:

			void Draw() override;

			void AddTriangleDraw(size_t count);

			void ResetTriangleDrawCount();

		private:
			friend class MainBar;

			size_t m_triangleDrawCount = 0;

		};
	}
}
