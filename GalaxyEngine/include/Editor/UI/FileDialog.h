#pragma once
#include "GalaxyAPI.h"
#include "EditorWindow.h"
#include "Utils/FileInfo.h"

namespace GALAXY 
{
	namespace Editor::UI
	{

		class TmpFile
		{
		public:
			TmpFile(const Path& path);
			TmpFile& operator=(const TmpFile& other) = default;
			TmpFile(const TmpFile&) = default;
			TmpFile(TmpFile&&) noexcept = default;
			virtual ~TmpFile();

			void FindChildrens();

			inline void SetSelected(Shared<TmpFile> file) 
			{ 
				if (m_selectedFile)
					m_selectedFile->m_selected = false;
				m_selectedFile = file; 
				m_selectedFile->m_selected = true;
			}
		private:
			friend class FileDialog;

			Utils::FileInfo m_info;

			Shared<Resource::Texture> m_icon;

			List<Shared<TmpFile>> m_childrens;

			Shared<TmpFile> m_selectedFile;

			bool m_selected = false;
		};

		enum class FileDialogType
		{
			Open,
			Save
		};

		class FileDialog : public EditorWindow
		{
		public:
			FileDialog();
			~FileDialog();

			void Draw(FileDialogType fileDialogType);

			void DrawPanel(ImGuiTextFilter& filter);

			inline Path GetCurrentPath() const { return m_currentPath; }
			void SetCurrentPath(Path val);
		private:
			Path m_currentPath = std::filesystem::current_path();

			Shared<TmpFile> m_currentFile;

			std::string m_search;
		};
	}
}
#include "Editor/UI/FileDialog.inl" 
