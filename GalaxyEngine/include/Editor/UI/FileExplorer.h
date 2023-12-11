#pragma once
#include "GalaxyAPI.h"
#include "Editor/UI/EditorWindow.h"

#include "Physic/2D/Utils.h"

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
			explicit File(const Path& path);
			File& operator=(const File& other) = default;
			File(const File&) = default;
			File(File&&) noexcept = default;
			virtual ~File() {}

			void FindChildren();
			void FindAllChildren();

			void DisplayOnExplorer() const;

		private:
			friend class FileExplorer;
			friend class FileDialog;

			Utils::FileInfo m_info;
			Weak<Resource::Texture> m_icon;
			Weak<Resource::IResource> m_resource;

			Weak<File> m_parent;
			List<Shared<File>> m_children;

			bool m_selected = false;
			bool m_isAnyChildFolder = false;
		};

		class FileExplorer : public EditorWindow
		{
		public:
			FileExplorer() = default;
			~FileExplorer() override = default;

			void Initialize() override;

			void Draw() override;

			void AddFileSelected(const Shared<File>& child);
			void RemoveFileSelected(const Shared<File>& child);

			void HandleDropFile(int count, const char** paths) const;

		private:
			void ClearSelected();

			void SetCurrentFile(const Shared<File>& file);

			void RightClickWindow();

			static void ShowInExplorer(const List<Shared<File>>& files, bool select);

			void ReloadContent() const;
		private:
			friend class MainBar;

			bool m_visible;

			Physic2D::Recti m_rect;

			Path m_workingDirectory;

			Shared<File> m_mainFile;
			Shared<File> m_currentFile;

			List<Shared<File>> m_selectedFiles;
			List<Shared<File>> m_rightClickedFiles;
			Shared<File> m_renameFile;
			bool m_openRename;
			std::string m_renameFileName;

			float m_iconSize = 86.0f;
			float m_space = 15.f;
		};
	}
}
#include "Editor/UI/FileExplorer.inl" 
