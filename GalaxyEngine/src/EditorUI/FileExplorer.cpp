#include "pch.h"
#include "EditorUI/FileExplorer.h"

#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"

#define FOLDER_ICON_PATH "CoreResources\\icons\\folder.png"
#define FILE_ICON_PATH "CoreResources\\icons\\file.png"

/* TODO:
* Drag and Drop (folders, models, ...)
* Delete
* Add folder
* Thumbnails
* Specification Right Click : (Ex. Recompile Shader)
* Bottom bar too see clicked file
* Inspector files selected
* All this need to work with multiple selected files
* Improve left child
 */

namespace fs = std::filesystem;
namespace GALAXY {
#pragma region File
	EditorUI::File::File(const std::filesystem::path& path)
	{
		m_resource = Resource::ResourceManager::GetInstance()->GetResource<Resource::IResource>(path);
		if (m_resource.lock())
			m_info = m_resource.lock()->GetFileInfo();
		else
			m_info = Utils::FileInfo(path);

		if (m_info.isDirectory())
		{
			m_icon = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(FOLDER_ICON_PATH);
			return;
		}
		using namespace Resource;
		switch (m_info.GetResourceType()) {
		case ResourceType::Texture:
		{
			m_icon = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(path);
			break;
		}
		/*
		case ResourceType::Shader:
		{
			break;
		}
		case ResourceType::VertexShader:
		{
			break;
		}
		case ResourceType::GeometryShader:
		{
			break;
		}
		case ResourceType::FragmentShader:
		{
			break;
		}
		case ResourceType::Model:
		{
			break;
		}
		case ResourceType::Mesh:
		{
			break;
		}
		case ResourceType::Material:
		{
			break;
		}
		*/
		default:
		{
			m_icon = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Texture>(FILE_ICON_PATH);
			break;
		}
		}
	}

	void EditorUI::File::FindChildrens()
	{
		auto dirIt = std::filesystem::directory_iterator(m_info.GetFullPath());
		for (const std::filesystem::directory_entry& entry : dirIt)
		{
			m_childrens.push_back(std::make_shared<File>(entry.path()));
			m_childrens.back()->m_parent = shared_from_this();
			if (entry.is_directory()) {
				m_isAnyChildFolder = true;
			}
		}
	}

	void EditorUI::File::FindAllChildrens()
	{
		if (!std::filesystem::exists(m_info.GetFullPath()))
		{
			PrintLog("%s does not exist, maybe because of no project path", m_info.GetFullPath().string().c_str());
			return;
		}
		auto dirIt = std::filesystem::directory_iterator(m_info.GetFullPath());
		for (const std::filesystem::directory_entry& entry : dirIt)
		{
			m_childrens.push_back(std::make_shared<File>(entry.path()));
			m_childrens.back()->m_parent = shared_from_this();
			if (entry.is_directory()) {
				m_isAnyChildFolder = true;
				m_childrens.back()->FindAllChildrens();
			}
		}
	}

	void EditorUI::File::DisplayOnExplorer()
	{
		if (m_info.isDirectory())
		{
			ImGui::BeginGroup();
			if (m_icon.lock()) {
				Wrapper::GUI::TextureImage(m_icon.lock().get(), 16);
				ImGui::SameLine();
			}
			if (ImGui::TreeNodeEx(m_info.GetFileName().string().c_str(), m_isAnyChildFolder ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf))
			{
				for (auto& children : m_childrens)
				{
					children->DisplayOnExplorer();
				}
			}
			ImGui::EndGroup();
		}
	}

#pragma endregion

	EditorUI::FileExplorer::FileExplorer()
	{
	}

	void EditorUI::FileExplorer::Initialize()
	{
		m_workingDirectory = Resource::ResourceManager::GetInstance()->GetAssetPath();
		m_mainFile = std::make_shared<File>(m_workingDirectory);
		m_mainFile->FindAllChildrens();

		m_currentFile = m_mainFile;
	}

	void EditorUI::FileExplorer::Draw()
	{
		if (!p_open) {
			return;
		}

		const int iconSize = 86;
		const int space = 15;
		const int textLength = 10;
		bool openRightClick = false;

		// Begin the ImGui window for the File Explorer
		if (ImGui::Begin("File Explorer", &p_open)) {

			static float size1 = 200, size2 = ImGui::GetContentRegionAvail().x;
			Wrapper::GUI::Splitter(true, 2, &size1, &size2, 10, 10);

			ImGui::BeginChild("Folder", Vec2f(size1, ImGui::GetContentRegionAvail().y));
			m_mainFile->DisplayOnExplorer();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Content", Vec2f(ImGui::GetContentRegionAvail().x, -1));
			if (ImGui::Button("Back") && m_currentFile->m_parent)
			{
				SetCurrentFile(m_currentFile->m_parent);
			}
			ImGui::SameLine();
			std::string in = m_currentFile->m_info.GetRelativePath().string();
			ImGui::InputText("search", &in);

			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Button, Vec4f(0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Vec4f(0, 0, 100, 255));

			// Iterate through each child of the current file
			for (size_t i = 0, j = 0; i < m_currentFile->m_childrens.size(); i++) {
				auto& child = m_currentFile->m_childrens[i];
				if (!child || !child->m_icon.lock() || child->m_info.GetResourceType() == Resource::ResourceType::Data) {
					continue;
				}

				ImGui::PushID((int)i);

				auto cursorPos = ImGui::GetCursorPos();

				// Handle file selection logic
				if (ImGui::Selectable("##select", &child->m_selected, ImGuiSelectableFlags_SelectOnClick, Vec2f(iconSize))) {
					if (child->m_selected || !child->m_selected && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
					{
						if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
						{
							ClearSelected();
						}
						AddFileSelected(child);
						child->m_selected = true;
					}
					else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
					{
						RemoveFileSelected(child);
					}
				}
				Vec2f selectedCursorPos = ImGui::GetCursorPos();

				// Handle double-click to open the file
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (child->m_info.isDirectory())
					{
						SetCurrentFile(child);
						break;
					}
				}
				else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					if (std::count(m_selectedFiles.begin(), m_selectedFiles.end(), child) == 0)
					{
						ClearSelected();
					}
					AddFileSelected(child);
					m_rightClickedFiles = m_selectedFiles;
					openRightClick = true;
				}
				else if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(child->m_info.GetFileName().string().c_str());
				}

				// Positioning for the file icon and text
				ImGui::SetCursorPos(cursorPos + Vec2f(12, 0));
				ImGui::BeginGroup();
				Wrapper::GUI::TextureImage(child->m_icon.lock().get(), Vec2f(iconSize - 24));

				// Truncate and display file name
				std::string fileName = child->m_info.GetFileName().string().substr(0, textLength + 3);
				if (child->m_info.GetFileName().string().length() > textLength + 3) {
					fileName.append("...");
				}
				Vec2f TextPos = Vec2f(-(ImGui::CalcTextSize(fileName.c_str()).x / 2) + iconSize / 2, iconSize - 24 + 5);
				ImGui::SetCursorPos(cursorPos + TextPos);
				ImGui::TextUnformatted(fileName.c_str());
				ImGui::EndGroup();

				if (ImGui::GetWindowWidth() - (j + 1) * (iconSize + space) > iconSize) {
					ImGui::SameLine(static_cast<float>((j + 1) * (iconSize + space)));
					j++;
				}
				else
				{
					j = 0;
					ImGui::Dummy(Vec2f{ 0, space / 2 + 2 });
				}

				ImGui::PopID();
			}
			ImGui::PopStyleColor(2);
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !openRightClick || openRightClick)
			{
				if (!openRightClick) {
					ClearSelected();
					m_rightClickedFiles.clear();
				}
				ImGui::OpenPopup("RightClickPopup");
			}
			RightClickWindow();
			ImGui::EndChild();
		}

		ImGui::End();

	}

	void EditorUI::FileExplorer::AddFileSelected(std::shared_ptr<File>& child)
	{
		if (std::count(m_selectedFiles.begin(), m_selectedFiles.end(), child) > 0)
			return;
		child->m_selected = true;
		m_selectedFiles.push_back(child);
	}

	void EditorUI::FileExplorer::RemoveFileSelected(std::shared_ptr<File>& child)
	{
		for (size_t i = 0; i < m_selectedFiles.size(); i++) {
			if (m_selectedFiles[i] == child) {
				m_selectedFiles.erase(m_selectedFiles.begin() + i);
				break;
			}
		}
	}

	void EditorUI::FileExplorer::ClearSelected()
	{
		for (auto& selectedFile : m_selectedFiles)
		{
			selectedFile->m_selected = false;
		}
		m_selectedFiles.clear();
	}

	void EditorUI::FileExplorer::SetCurrentFile(std::shared_ptr<File>& file)
	{
		m_currentFile = file;
		ClearSelected();
	}

	void EditorUI::FileExplorer::RightClickWindow()
	{
		if (ImGui::BeginPopup("RightClickPopup"))
		{
			static auto quitPopup = [this]()
			{
				m_rightClickedFiles.clear();
				ImGui::CloseCurrentPopup();
			};

			Vec2f buttonSize = Vec2f(ImGui::GetWindowContentRegionWidth(), 0);
			if (!m_rightClickedFiles.empty()) {
				ImGui::TextUnformatted(m_rightClickedFiles[0]->m_info.GetFileName().string().c_str());
				buttonSize = Vec2f(ImGui::GetWindowContentRegionWidth(), 0);
				if (m_rightClickedFiles[0]->m_resource.lock()) {
					bool allSame = true;
					Resource::ResourceType commonType = m_rightClickedFiles[0]->m_resource.lock()->GetFileInfo().GetResourceType();

					for (auto& file : m_rightClickedFiles)
					{
						if (!file->m_resource.lock() || commonType != file->m_resource.lock()->GetFileInfo().GetResourceType())
						{
							allSame = false;
						}
					}
					if (allSame) {
						switch (commonType)
						{
						case Resource::ResourceType::Texture:
							break;
						case Resource::ResourceType::Shader:
							break;
						case Resource::ResourceType::VertexShader:
							break;
						case Resource::ResourceType::GeometryShader:
							break;
						case Resource::ResourceType::FragmentShader:
							break;
						case Resource::ResourceType::Model:
							if (ImGui::Button("Import", buttonSize))
							{
								for (auto& file : m_rightClickedFiles)
								{
									file->m_resource.lock()->Load();
								}
							}
							break;
						case Resource::ResourceType::Mesh:
							break;
						case Resource::ResourceType::Material:
							break;
						default:
							break;
						}
					}
				}

				if (ImGui::Button("Delete", buttonSize))
				{

					quitPopup();
				}
				ImGui::Separator();

			}
			if (ImGui::Button("Show In Explorer", buttonSize))
			{
				ShowInExplorer(!m_rightClickedFiles.empty() ?
					m_rightClickedFiles :
					std::vector<std::shared_ptr<File>>{ m_currentFile }, !m_rightClickedFiles.empty());
				quitPopup();
			}
			if (ImGui::Button("New Folder", buttonSize))
			{

				quitPopup();
			}
			ImGui::EndPopup();
		}
	}
#ifdef _WIN32
#include <shlobj_core.h>
#endif

	void EditorUI::FileExplorer::ShowInExplorer(const std::vector<std::shared_ptr<File>>& files, bool select)
	{
#ifdef _WIN32
		
		const char *explorerPath = "explorer.exe";

		// Construct the command
		const char *command = select ? "/select,\"" : "\"";
		char fullCommand[MAX_PATH + sizeof(command) + 2];
		snprintf(fullCommand, sizeof(fullCommand), "%s%s\"", command, files[0]->m_info.GetFullPath().string().c_str());

		// Launch File Explorer
		HINSTANCE result = ShellExecute(nullptr, "open", explorerPath, fullCommand, nullptr, SW_SHOWNORMAL);

		if ((intptr_t)result <= 32) {
			DWORD error = GetLastError();
			PrintError("Failed to Open Explorer (Error Code: %lu)", error);
		}
		
			/*
		const char* explorerPath = "explorer.exe";

		// Construct the command
		std::string command = select ? "/select,\"" : "\"";
		for (auto& file : files) {
			command += file->m_info.GetFullPath().string() + ",";
		}
		command.pop_back();  // Remove the trailing comma
		command += "\"";

		// Launch File Explorer
		HINSTANCE result = ShellExecute(nullptr, "open", explorerPath, command.c_str(), nullptr, SW_SHOWNORMAL);

		if ((intptr_t)result <= 32) {
			DWORD error = GetLastError();
			PrintError("Failed to Open Explorer (Error Code: %lu)", error);
			// You might also use FormatMessage to get a more detailed error description
		}
		*/
#else
		//TODO
#endif
	}

}