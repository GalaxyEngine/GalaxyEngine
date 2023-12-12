#include "pch.h"
#include "Editor/UI/FileExplorer.h"
#include "Editor/UI/EditorUIManager.h"

#include "Core/Application.h"
#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/Script.h"
#include "Resource/Material.h"
#include "Resource/Model.h"
#include "Wrapper/Window.h"

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
		static FileExplorer* explorer = EditorUIManager::GetInstance()->GetFileExplorer();
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

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE")) {

					auto draggedFiles = explorer->m_draggedFiles;
					for (int i = 0; i < draggedFiles.size(); ++i) {

						Path oldPath = draggedFiles[i]->m_info.GetFullPath();

						if (oldPath == m_info.GetFullPath())
							continue;

						Path newPath = m_info.GetFullPath() / oldPath.filename();

						// Rename Resource in resourceManager and file explorer
						Resource::ResourceManager::HandleRename(oldPath, newPath);
					}
					explorer->ReloadContent();
				}
				ImGui::EndDragDropTarget();
			}

		}
	}

	Shared<Editor::UI::File> Editor::UI::File::GetWithPath(const Path& path) const
	{
		for (const Shared<File>& child : m_children)
		{
			if (child->m_info.GetFullPath() == path)
				return child;
			else {
				Shared<File> result = child->GetWithPath(path);
				if (result)
					return result;
			}
		}
		return nullptr;
	}

#pragma endregion

	void Editor::UI::FileExplorer::Initialize()
	{
		m_workingDirectory = Resource::ResourceManager::GetInstance()->GetAssetPath();
		m_mainFile = std::make_shared<File>(m_workingDirectory);
		m_mainFile->FindAllChildren();

		m_currentFile = m_mainFile;

		m_iconSize = m_iconSize * Wrapper::GUI::GetScaleFactor();

		EditorUIManager::GetInstance()->GetInspector()->SetFileSelected(&m_selectedFiles);
	}

	void Editor::UI::FileExplorer::Draw()
	{
		if (!p_open)
			return;

		constexpr float minIconSize = 32.f;
		constexpr float maxIconSize = 200.f;

		constexpr float iconDeltaZoom = 5.f;
		constexpr int textLength = 9;
		bool openRightClick = false;

		// Begin the ImGui window for the File Explorer
		if (m_visible = ImGui::Begin("File Explorer", &p_open); m_visible) {
			const Vec2f vMin = ImGui::GetWindowContentRegionMin();
			const Vec2f vMax = ImGui::GetWindowContentRegionMax();
			const Vec2f windowPos = Vec2f(ImGui::GetWindowPos());

			m_rect.min = vMin + windowPos;
			m_rect.max = vMax + windowPos;

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
				FileInfo& info = child->m_info;
				if (!child || !child->m_icon.lock() || info.GetResourceType() == Resource::ResourceType::Data) {
					continue;
				}

				ImGui::PushID(static_cast<int>(i));

				auto cursorPos = ImGui::GetCursorPos();

				// Handle file selection logic
				if (ImGui::Selectable("##select", &child->m_selected, ImGuiSelectableFlags_SelectOnNav, Vec2f(m_iconSize))) {
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

				DragAndDrop(child);

				// Handle double-click to open the file
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (info.isDirectory())
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
					ImGui::SetTooltip(info.GetFileName().c_str());
				}

				// Positioning for the file icon and text
				ImGui::SetCursorPos(cursorPos + Vec2f(12, 0));
				ImGui::BeginGroup();
				Wrapper::GUI::TextureImage(child->m_icon.lock().get(), Vec2f(m_iconSize - 24.f));
				if (m_renameFile != child) {
					// Truncate and display file name
					const size_t length = info.GetFileName().length();
					std::string fileName = info.GetFileName();
					if (length > textLength + 3) {
						fileName = fileName.substr(0, textLength);
						fileName.append("...");
					}

					Vec2f TextPos = Vec2f(-(ImGui::CalcTextSize(fileName.c_str()).x / 2.f) + m_iconSize / 2.f, m_iconSize - 24.f + 5.f);
					ImGui::SetCursorPos(cursorPos + TextPos);
					ImGui::TextUnformatted(fileName.c_str());
				}
#pragma region Rename
				else
				{
					// Display input text for rename
					if (m_openRename)
					{
						ImGui::SetKeyboardFocusHere();
					}
					Vec2f TextPos = { 0, m_iconSize - 24.f + 5.f };
					ImGui::SetCursorPos(cursorPos + TextPos);
					ImGui::SetNextItemWidth(m_iconSize);
					bool enter = Wrapper::GUI::InputText("##InputText", &m_renameFileName, ImGuiInputTextFlags_EnterReturnsTrue);
					if (m_renameFile && !m_openRename && enter)
					{
						const Path oldPath = m_renameFile->m_info.GetFullPath();
						const Path newPath = m_renameFile->m_info.GetFullPath().parent_path()
							/ (m_renameFileName + m_renameFile->m_info.GetExtension().string());

						// Rename Resource in resourceManager and file explorer
						Resource::ResourceManager::HandleRename(oldPath, newPath);

						ReloadContent();
					}
					if (m_renameFile && !m_openRename && !ImGui::IsItemActive())
					{
						m_renameFile = nullptr;
						m_renameFileName = "";
					}
					m_openRename = false;
				}
#pragma endregion
				ImGui::EndGroup();

				if (ImGui::GetWindowWidth() - (j + 1) * (m_iconSize + m_space) > m_iconSize) {
					ImGui::SameLine(static_cast<float>((j + 1) * (m_iconSize + m_space)));
					j++;
				}
				else
				{
					j = 0;
					ImGui::Dummy(Vec2f{ 0, m_space / 2.f + 2.f });
				}

				ImGui::PopID();
			}
			ImGui::PopStyleColor(2);
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::GetIO().MouseWheel != 0)
			{
				m_iconSize += ImGui::GetIO().MouseWheel * iconDeltaZoom;
				m_iconSize = std::clamp(m_iconSize, minIconSize, maxIconSize);
				m_space = ((1 / m_iconSize) * 86.f) * 60.f;
			}
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
			UpdateReloadContent();
		}

		ImGui::End();

	}

	void Editor::UI::FileExplorer::AddFileSelected(const Shared<File>& child)
	{
		if (std::ranges::count(m_selectedFiles, child) > 0)
			return;
		child->m_selected = true;
		m_selectedFiles.push_back(child);
		EditorUIManager::GetInstance()->GetInspector()->UpdateFileSelected();
	}

	void Editor::UI::FileExplorer::RemoveFileSelected(const Shared<File>& child)
	{
		for (size_t i = 0; i < m_selectedFiles.size(); i++) {
			if (m_selectedFiles[i] == child) {
				m_selectedFiles.erase(m_selectedFiles.begin() + i);
				break;
			}
		}
		EditorUIManager::GetInstance()->GetInspector()->UpdateFileSelected();
	}

	void Editor::UI::FileExplorer::HandleDropFile(const int count, const char** paths) const
	{
		static auto window = Core::Application::GetInstance().GetWindow();
		if (!p_open || !m_visible)
			return;

		const Vec2f mousePos = window->GetMousePosition(Wrapper::CoordinateSpace::Screen);

		if (!m_rect.IsPointInside(mousePos))
			return;

		for (size_t i = 0; i < count; i++)
		{
			PrintLog("Dropped file: %s", paths[i]);
			/* TODO:
			 *	Copy files in file explorer (current directory if not folder selected, else inside the folder)
			 *	Load the files dropped
			 *	(maybe add a file watcher to check at any time if a file was added inside the asset directory)
			*/
		}
	}

	void Editor::UI::FileExplorer::ClearSelected()
	{
		for (const auto& selectedFile : m_selectedFiles)
		{
			selectedFile->m_selected = false;
		}
		m_selectedFiles.clear();
		EditorUIManager::GetInstance()->GetInspector()->UpdateFileSelected();
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
							if (m_rightClickedFiles.size() == 1 && m_rightClickedFiles[0]->m_resource.lock()->IsLoaded())
								break;
							if (ImGui::Button("Load", buttonSize))
							{
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									Resource::ResourceManager::GetOrLoad<Resource::Model>(file->m_info.GetFullPath());
								}
							}
							break;
						case Resource::ResourceType::Mesh:
							break;
						case Resource::ResourceType::Material:
							if (m_rightClickedFiles.size() == 1 && m_rightClickedFiles[0]->m_resource.lock()->IsLoaded())
								break;
							if (ImGui::Button("Load", buttonSize))
							{
								for (const Shared<File>& file : m_rightClickedFiles)
								{
									Resource::ResourceManager::GetOrLoad<Resource::Material>(file->m_info.GetFullPath());
								}
							}
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

				if (ImGui::Button("Rename", buttonSize))
				{
					m_renameFile = m_rightClickedFiles[0];
					m_openRename = true;
					m_renameFileName = m_renameFile->m_info.GetFileNameNoExtension();
					quitPopup();
				}
				ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
				if (ImGui::Button("Delete", buttonSize))
				{
					for (const Shared<File>& file : m_rightClickedFiles)
					{
						// Remove Resource file
						Resource::ResourceManager::GetInstance()->RemoveResource(file->m_info.GetRelativePath());
						std::remove(file->m_info.GetFullPath().string().c_str());
						std::remove((file->m_info.GetFullPath().string() + ".gdata").c_str());
					}
					quitPopup();
					ReloadContent();
				}
				ImGui::PopStyleColor();
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
		// TODO : Move to OS Specific 

		const char* explorerPath = "explorer.exe";

		// Construct the command
		const char* command = select ? "/select,\"" : "\"";
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

	void Editor::UI::FileExplorer::ReloadContent()
	{
		m_shouldReloadContent = true;
	}

	void Editor::UI::FileExplorer::UpdateReloadContent()
	{
		if (!m_shouldReloadContent)
			return;
		m_shouldReloadContent = false;
		auto currentPath = m_currentFile->m_info.GetFullPath();
		m_mainFile->m_children.clear();
		m_mainFile->FindAllChildren();
		m_currentFile = m_mainFile->GetWithPath(currentPath);
		if (!m_currentFile)
			m_currentFile = m_mainFile;
	}

	void Editor::UI::FileExplorer::DragAndDrop(Shared<File>& child)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			m_draggedFiles.clear();
			if (!child->m_selected)
				m_draggedFiles.push_back(child);
			for (int i = 0; i < m_selectedFiles.size(); ++i) {
				m_draggedFiles.push_back(m_selectedFiles[i]);
			}
			for (int i = 0; i < m_draggedFiles.size(); i++)
			{
				ImGui::TextUnformatted(m_draggedFiles[i]->m_info.GetFileName().c_str());
			}
			ImGui::SetDragDropPayload("FILE", nullptr, 0);
			ImGui::EndDragDropSource();
		}
		if (child->m_info.isDirectory())
		{
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE")) {

					for (int i = 0; i < m_draggedFiles.size(); ++i) {
						Path oldPath = m_draggedFiles[i]->m_info.GetFullPath();
						Path newPath = child->m_info.GetFullPath() / oldPath.filename();

						// Rename Resource in resourceManager
						Resource::ResourceManager::HandleRename(oldPath, newPath);
					}
					ReloadContent();
				}
				ImGui::EndDragDropTarget();
			}
		}
	}

}