#include "pch.h"
#include "Editor/UI/FileExplorer.h"

#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/Script.h"
#include "Resource/Material.h"

/* TODO:
* Drag and Drop (folders, models, ...)
* Delete
* Rename
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
	Editor::UI::File::File(const Path& path)
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

	void Editor::UI::File::FindChildren()
	{
		const auto dirIt = std::filesystem::directory_iterator(m_info.GetFullPath());
		for (const std::filesystem::directory_entry& entry : dirIt)
		{
			m_children.push_back(std::make_shared<File>(entry.path()));
			m_children.back()->m_parent = shared_from_this();
			if (entry.is_directory()) {
				m_isAnyChildFolder = true;
			}
		}
	}

	void Editor::UI::File::FindAllChildren()
	{
		if (!std::filesystem::exists(m_info.GetFullPath()))
		{
			PrintLog("%s does not exist, maybe because of no project path", m_info.GetFullPath().string().c_str());
			return;
		}
		const auto dirIt = std::filesystem::directory_iterator(m_info.GetFullPath());
		for (const std::filesystem::directory_entry& entry : dirIt)
		{
			m_children.push_back(std::make_shared<File>(entry.path()));
			m_children.back()->m_parent = weak_from_this();
			if (entry.is_directory()) {
				m_isAnyChildFolder = true;
				m_children.back()->FindAllChildren();
			}
		}
	}

	void Editor::UI::File::DisplayOnExplorer() const
	{
		if (m_info.isDirectory())
		{
			ImGui::BeginGroup();
			if (m_icon.lock()) {
				Wrapper::GUI::TextureImage(m_icon.lock().get(), Vec2f(16));
				ImGui::SameLine();
			}
			if (ImGui::TreeNodeEx(m_info.GetFileName().c_str(), m_isAnyChildFolder ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf))
			{
				for (const Shared<File>& children : m_children)
				{
					children->DisplayOnExplorer();
				}
			}
			ImGui::EndGroup();
		}
	}

#pragma endregion

	void Editor::UI::FileExplorer::Initialize()
	{
		m_workingDirectory = Resource::ResourceManager::GetInstance()->GetAssetPath();
		m_mainFile = std::make_shared<File>(m_workingDirectory);
		m_mainFile->FindAllChildren();

		m_currentFile = m_mainFile;
	}

	void Editor::UI::FileExplorer::Draw()
	{
		if (!p_open)
			return;

		const float iconSize = 86 * Wrapper::GUI::GetScaleFactor();
		constexpr int space = 15;
		constexpr int textLength = 9;
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
			if (ImGui::Button("Back") && m_currentFile->m_parent.lock())
			{
				Shared<GALAXY::Editor::UI::File> parent = m_currentFile->m_parent.lock();
				SetCurrentFile(parent);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload"))
			{
				ReloadContent();
			}
			ImGui::SameLine();
			std::string in = m_currentFile->m_info.GetRelativePath().string();
			Wrapper::GUI::InputText("search", &in);

			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Button, Vec4f(0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Vec4f(0, 0, 100, 255));

			// Iterate through each child of the current file
			for (size_t i = 0, j = 0; i < m_currentFile->m_children.size(); i++) {
				Shared<File>& child = m_currentFile->m_children[i];
				if (!child || !child->m_icon.lock() || child->m_info.GetResourceType() == Resource::ResourceType::Data) {
					continue;
				}

				ImGui::PushID(static_cast<int>(i));

				auto cursorPos = ImGui::GetCursorPos();

				// Handle file selection logic
				if (ImGui::Selectable("##select", &child->m_selected, ImGuiSelectableFlags_SelectOnClick, Vec2f(iconSize))) {
					if (child->m_selected || (!child->m_selected && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl)))
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
					if (std::ranges::count(m_selectedFiles, child) == 0)
					{
						ClearSelected();
					}
					AddFileSelected(child);
					m_rightClickedFiles = m_selectedFiles;
					openRightClick = true;
				}
				else if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(child->m_info.GetFileName().c_str());
				}

				// Positioning for the file icon and text
				ImGui::SetCursorPos(cursorPos + Vec2f(12, 0));
				ImGui::BeginGroup();
				Wrapper::GUI::TextureImage(child->m_icon.lock().get(), Vec2f(iconSize - 24.f));

				// Truncate and display file name
				const size_t length = child->m_info.GetFileName().length();
				std::string fileName = child->m_info.GetFileName();
				if (length > textLength + 3) {
					fileName = fileName.substr(0, textLength);
					fileName.append("...");
				}
				Vec2f TextPos = Vec2f(-(ImGui::CalcTextSize(fileName.c_str()).x / 2.f) + iconSize / 2.f, iconSize - 24.f + 5.f);
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

	void Editor::UI::FileExplorer::AddFileSelected(const Shared<File>& child)
	{
		if (std::ranges::count(m_selectedFiles, child) > 0)
			return;
		child->m_selected = true;
		m_selectedFiles.push_back(child);
	}

	void Editor::UI::FileExplorer::RemoveFileSelected(const Shared<File>& child)
	{
		for (size_t i = 0; i < m_selectedFiles.size(); i++) {
			if (m_selectedFiles[i] == child) {
				m_selectedFiles.erase(m_selectedFiles.begin() + i);
				break;
			}
		}
	}

	void Editor::UI::FileExplorer::ClearSelected()
	{
		for (const auto& selectedFile : m_selectedFiles)
		{
			selectedFile->m_selected = false;
		}
		m_selectedFiles.clear();
	}

	void Editor::UI::FileExplorer::SetCurrentFile(const Shared<File>& file)
	{
		m_currentFile = file;
		ClearSelected();
	}

	void Editor::UI::FileExplorer::RightClickWindow()
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
				ImGui::TextUnformatted(m_rightClickedFiles[0]->m_info.GetFileName().c_str());
				buttonSize = Vec2f(ImGui::GetWindowContentRegionWidth(), 0);
				if (m_rightClickedFiles[0]->m_resource.lock()) {
					bool allSame = true;
					const Resource::ResourceType commonType = m_rightClickedFiles[0]->m_resource.lock()->GetFileInfo().GetResourceType();

					for (const Shared<File>& file : m_rightClickedFiles)
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
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									file->m_resource.lock()->Load();
								}
							}
							break;
						case Resource::ResourceType::Mesh:
							break;
						case Resource::ResourceType::Material:
							break;
						case Resource::ResourceType::Script:
							if (ImGui::Button("Edit", buttonSize))
							{
								Resource::Script::OpenScript(m_rightClickedFiles[0]->m_info.GetFullPath());
							}
							break;
						default:
							break;
						}
					}
				}

				if (ImGui::Button("Delete", buttonSize))
				{
					for (const Shared<File>& file : m_rightClickedFiles)
					{
						Resource::ResourceManager::GetInstance()->RemoveResource(file->m_info.GetRelativePath());
						std::remove(file->m_info.GetFullPath().string().c_str());
					}
					quitPopup();
					ReloadContent();
				}
				ImGui::Separator();

			}
			if (ImGui::Button("Show In Explorer", buttonSize))
			{
				ShowInExplorer(!m_rightClickedFiles.empty() ?
					m_rightClickedFiles :
					std::vector<Shared<File>>{ m_currentFile }, !m_rightClickedFiles.empty());
				quitPopup();
			}
			if (ImGui::Button("New Folder", buttonSize))
			{
				std::filesystem::create_directory(m_currentFile->m_info.GetFullPath() / "New Folder");
				ReloadContent();
				quitPopup();
			}
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::Button("Script", buttonSize))
				{
					ImGui::OpenPopup("Create Script");
				}
			if (ImGui::BeginPopupModal("Create Script"))
			{
				static std::string scriptName;
				Wrapper::GUI::InputText("Script Name", &scriptName);
				if (ImGui::Button("Create") && !scriptName.empty())
				{
					Resource::Script::Create(m_currentFile->m_info.GetFullPath() / scriptName);

					ReloadContent();
					ImGui::CloseCurrentPopup();
					quitPopup();
				}
				ImGui::EndPopup();
			}
				if (ImGui::Button("Material", buttonSize))
				{
					Resource::Material::Create(m_currentFile->m_info.GetFullPath() / "New Material.mat");
					ReloadContent();
					quitPopup();
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}

	void Editor::UI::FileExplorer::ShowInExplorer(const std::vector<Shared<File>>& files, const bool select)
	{
#ifdef _WIN32
		
		const char *explorerPath = "explorer.exe";

		// Construct the command
		const char *command = select ? "/select,\"" : "\"";
		char fullCommand[MAX_PATH + sizeof(command) + 2];
		snprintf(fullCommand, sizeof(fullCommand), "%s%s\"", command, files[0]->m_info.GetFullPath().string().c_str());

		// Launch File Explorer

		if (HINSTANCE result = ShellExecute(nullptr, "open", explorerPath, fullCommand, nullptr, SW_SHOWNORMAL); reinterpret_cast<intptr_t>(result) <= 32) {
			const DWORD error = GetLastError();
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
#elif defined(__linux__)
		std::string command = "xdg-open ";
		std::string fullCommand;
		if (files[0]->m_info.isDirectory())
			fullCommand = command + files[0]->m_info.GetFullPath().generic_string() + "/";
		else
			// if it's a file, we open the parent folder (because i cannot find how to open file explorer and select a file with xdg (TODO))
			fullCommand = command + files[0]->m_info.GetFullPath().parent_path().generic_string() + "/";
		if (std::system(fullCommand.c_str()) != 0) {
			std::perror("Failed to open file explorer");
			// Handle error as needed
		}
		
#endif
	}

	void Editor::UI::FileExplorer::ReloadContent() const
	{
		m_currentFile->m_children.clear();
		m_currentFile->FindAllChildren();
	}
}