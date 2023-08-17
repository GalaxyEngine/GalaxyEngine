#pragma once
#include "GalaxyAPI.h"
#include "EditorUI/EditorWindow.h"

#include "Utils/FileInfo.h"

#include <filesystem>
namespace GALAXY 
{
	namespace Resource
	{
		class Texture;
	}
	namespace EditorUI
	{
		class File : public std::enable_shared_from_this<File>
		{
		public:
			File(const std::filesystem::path& path);
			File& operator=(const File& other) = default;
			File(const File&) = default;
			File(File&&) noexcept = default;
			virtual ~File() {}

			void FindChildrens();
			void FindAllChildrens();

			void DisplayOnExplorer();

		private:
			friend class FileExplorer;

			Utils::FileInfo m_info;
			std::weak_ptr<Resource::Texture> m_icon;

			std::shared_ptr<File> m_parent;
			std::vector<std::shared_ptr<File>> m_childrens;

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

			void AddFileSelected(std::shared_ptr<File>& child);
			void RemoveFileSelected(std::shared_ptr<File>& child);

		private:
			void ClearSelected();
			void SetCurrentFile(std::shared_ptr<File>& file);

			void RightClickWindow();

			void ShowInExplorer(const std::vector<std::shared_ptr<File>>& files, bool select);
		private:
			std::filesystem::path m_workingDirectory = ASSET_FOLDER_NAME;

			std::shared_ptr<File> m_mainFile;
			std::shared_ptr<File> m_currentFile;

			std::vector<std::shared_ptr<File>> m_selectedFiles;
			std::vector<std::shared_ptr<File>> m_rightClickedFiles ;
		};
	}
}
#include "EditorUI/FileExplorer.inl" 
