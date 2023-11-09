#pragma once
#include "GalaxyAPI.h"
#include "Editor/UI/EditorWindow.h"

#include "Utils/FileInfo.h"

#include <filesystem>
namespace GALAXY
{
	namespace Resource
	{
		class Texture;
	}
	namespace Editor::UI
	{
		class File : public std::enable_shared_from_this<File>
		{
		public:
			File(const Path& path);
			File& operator=(const File& other) = default;
			File(const File&) = default;
			File(File&&) noexcept = default;
			virtual ~File() {}

			void FindChildrens();
			void FindAllChildrens();

			void DisplayOnExplorer();

		private:
			friend class FileExplorer;
			friend class FileDialog;

			Utils::FileInfo m_info;
			Weak<Resource::Texture> m_icon;
			Weak<Resource::IResource> m_resource;

			Weak<File> m_parent;
			List<Shared<File>> m_childrens;

			bool m_selected = false;
			bool m_isAnyChildFolder = false;
		};

		class FileExplorer : public EditorWindow
		{
		public:
			FileExplorer();
			~FileExplorer() {}

			void Initialize() override;

			void Draw() override;

			void AddFileSelected(Shared<File>& child);
			void RemoveFileSelected(Shared<File>& child);

		private:
			void ClearSelected();

			void SetCurrentFile(Shared<File>& file);

			void RightClickWindow();

			void ShowInExplorer(const List<Shared<File>>& files, bool select);

			void ReloadContent();
		private:
			friend class MainBar;

			Path m_workingDirectory = ASSET_FOLDER_NAME;

			Shared<File> m_mainFile;
			Shared<File> m_currentFile;

			List<Shared<File>> m_selectedFiles;
			List<Shared<File>> m_rightClickedFiles;
		};
	}
}
#include "Editor/UI/FileExplorer.inl" 
