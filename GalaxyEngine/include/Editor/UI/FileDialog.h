#pragma once
#include "GalaxyAPI.h"
#include "EditorWindow.h"
#include "Utils/FileInfo.h"

namespace GALAXY 
{
	namespace Editor::UI
	{
		class FileDialog : public EditorWindow
		{
		public:

			void Draw();

			inline Path GetCurrentPath() const { return m_currentPath; }
			inline void SetCurrentPath(Path val) { m_currentPath = val; }
		private:
			Path m_currentPath = std::filesystem::current_path();

		};
	}
}
#include "Editor/UI/FileDialog.inl" 
