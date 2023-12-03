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
			explicit TmpFile(const Path& path);
			TmpFile& operator=(const TmpFile& other) = default;
			TmpFile(const TmpFile&) = default;
			TmpFile(TmpFile&&) noexcept = default;
			virtual ~TmpFile() = default;

			void FindChildren();

			inline void SetSelected(const Shared<TmpFile>& file) 
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

			List<Shared<TmpFile>> m_children;

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
			~FileDialog() override = default;

			void Draw() override;

			void Exit();

			void DrawPanel(const ImGuiTextFilter& textFilter);

			inline Path GetCurrentPath() const { return m_currentPath; }

			void SetCurrentPath(const Path& val);
			inline void SetInitialized(const bool val) { m_initialized = val; }

			inline void SetFileDialogType(const FileDialogType val) { m_fileDialogType = val; }
			inline std::string GetOutput() const { return m_output; }
			inline FileDialogType GetFileDialogType() const { return m_fileDialogType; }

			static void OpenFileDialog(FileDialogType fileDialogType, std::string filter = "", const Path& initialDir = std::filesystem::current_path());

			static bool Initialized();
		private:
			Path m_currentPath = std::filesystem::current_path();

			FileDialogType m_fileDialogType = FileDialogType::Open;

			Shared<TmpFile> m_currentFile;

			std::string m_search;

			std::string m_output;

			bool m_initialized = false;

			std::string m_filter;

			std::string m_fileName;

			bool m_exited = false;
		};
	}
}
#include "Editor/UI/FileDialog.inl" 
