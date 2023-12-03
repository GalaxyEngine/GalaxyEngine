#include "pch.h"

#include "Utils/Define.h"

#include "Editor/UI/FileDialog.h"
#include "Editor/UI/FileExplorer.h"
#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

namespace GALAXY
{
#pragma region TmpFile
	Editor::UI::TmpFile::TmpFile(const Path& path)
	{
		m_info = Utils::FileInfo(path, false);

		// TODO: Load only visible thumbnail
		if (m_info.isDirectory())
		{
			m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(FOLDER_ICON_PATH).lock();
		}
		else if (m_info.GetResourceType() == Resource::ResourceType::Texture)
		{
			m_icon = Resource::ResourceManager::TemporaryLoad<Resource::Texture>(m_info.GetFullPath());
		}
		else
		{
			m_icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(FILE_ICON_PATH).lock();
		}
	}

	void Editor::UI::TmpFile::FindChildren()
	{
		try {
			const auto iterator = std::filesystem::directory_iterator(m_info.GetFullPath());
			for (const auto& entry : iterator)
			{
				Shared<TmpFile> child = std::make_shared<TmpFile>(entry.path());
				m_children.push_back(child);
			}
		}
		catch (const std::exception&)
		{
			PrintError("Failed to find childrens files of %s", m_info.GetFullPath().string().c_str());
		}
	}

#pragma endregion

	Editor::UI::FileDialog::FileDialog()
	{
		SetOpen(false);
	}

	void Editor::UI::FileDialog::Draw()
	{
		if (!p_open)
			return;

		if (ImGui::Begin("File Dialog", &p_open))
		{
			if (ImGui::Button("Refresh"))
			{
				SetCurrentPath(m_currentPath);
			}
			ImGui::SameLine();
			if (ImGui::Button("<|"))
			{
				const Path parentPath = m_currentPath.parent_path();
				if (std::filesystem::exists(parentPath))
					SetCurrentPath(parentPath);
			}
			ImGui::SameLine();
			std::string path = m_currentPath.generic_string();
			if (Wrapper::GUI::InputText("##Path", &path, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (std::filesystem::exists(path))
					SetCurrentPath(path);
			}

			ImGui::SameLine();

			static ImGuiTextFilter textFilter;
			textFilter.Draw("Search");

			DrawPanel(textFilter);
			ImGui::Separator();

			Wrapper::GUI::InputText("File Name", &m_fileName);
			ImGui::SameLine();
			bool buttonClicked = false;
			switch (m_fileDialogType)
			{
			case FileDialogType::Open:
				buttonClicked = ImGui::Button("Open");
				break;
			case FileDialogType::Save:
				buttonClicked = ImGui::Button("Save");
				break;
			default:
				break;
			}
			if (buttonClicked)
				Exit();

			if (!m_filter.empty()) {
				ImGui::TextUnformatted(m_filter.c_str());
			}
		}
		ImGui::End();

		if (!p_open && !m_exited)
		{
			m_initialized = false;
			m_output = "";
			m_fileName = "";
		}
		else if (m_exited)
		{
			m_exited = false;
		}
	}

	void Editor::UI::FileDialog::SetCurrentPath(const Path& val)
	{
		if (std::filesystem::exists(val))
		{
			m_currentPath = val;
		}
		else
		{
			m_currentPath = std::filesystem::current_path();
		}
		m_currentFile = std::make_shared<TmpFile>(m_currentPath);
		m_currentFile->FindChildren();
	}

	void Editor::UI::FileDialog::DrawPanel(const ImGuiTextFilter& textFilter)
	{
		ImGui::BeginChild("Content Panel", ImGui::GetContentRegionAvail() - Vec2f(0, 50));

		constexpr int iconSize = 86;
		constexpr int textLength = 10;
		constexpr int space = 15;

		static ImGuiTextFilter filterExtension;
		if (!m_filter.empty()) {
			const size_t count = std::min(m_filter.size(), sizeof(filterExtension.InputBuf) - 1);
#ifdef _WIN32
			strncpy_s(filterExtension.InputBuf, m_filter.c_str(), count);
#else
			strncpy(filterExtension.InputBuf, m_filter.c_str(), count);
    		filterExtension.InputBuf[count] = '\0';  // Ensure null-termination
#endif
			filterExtension.InputBuf[count] = '\0';  // Ensure null-termination
			filterExtension.Build();
		}
		for (size_t i = 0, j = 0; i < m_currentFile->m_children.size(); i++)
		{
			const auto& file = m_currentFile->m_children[i];

			const bool passExtensionFilter = (!filterExtension.PassFilter(file->m_info.GetExtension().string().c_str()) && !file->m_info.isDirectory());
			if (!textFilter.PassFilter(file->m_info.GetFileName().c_str()) || passExtensionFilter)
				continue;

			ImGui::PushID(static_cast<int>(i));

			Vec2f cursorPos = ImGui::GetCursorPos();

			// Handle file selection logic
			if (ImGui::Selectable("##select", &file->m_selected, ImGuiSelectableFlags_SelectOnClick, Vec2f(iconSize))) {
				if (file->m_selected || (!file->m_selected && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl)))
				{
					m_currentFile->SetSelected(file);
					if (!file->m_info.isDirectory())
						m_fileName = file->m_info.GetFileName();
				}
			}

			// Handle double-click to open the file
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (file->m_info.isDirectory())
				{
					SetCurrentPath(file->m_info.GetFullPath());
					break;
				}
				else
				{
					Exit();
				}
			}
			else if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(file->m_info.GetFileName().c_str());
			}

			// Positioning for the file icon and text
			ImGui::SetCursorPos(cursorPos + Vec2f(12, 0));

			ImGui::BeginGroup();
			Wrapper::GUI::TextureImage(file->m_icon.get(), Vec2f(iconSize - 24));

			// Truncate and display file name
			const size_t length = file->m_info.GetFileName().length();
			std::string fileName = file->m_info.GetFileName();
			if (length > textLength + 3) {
				fileName = fileName.substr(0, textLength);
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
		ImGui::EndChild();
	}

	void Editor::UI::FileDialog::Exit()
	{
		const std::string finalFilePath = (m_currentFile->m_info.GetFullPath() / m_fileName).generic_string();
		m_output = finalFilePath;
		SetOpen(false);
		m_exited = true;
	}

	void Editor::UI::FileDialog::OpenFileDialog(const FileDialogType fileDialogType, std::string filter /*= ""*/, const Path& initialDir /*= std::filesystem::current_path()*/)
	{
		static Editor::UI::FileDialog* fileDialog = Editor::UI::EditorUIManager::GetInstance()->GetFileDialog();
		// Refresh file dialog
		if (!fileDialog->m_initialized) {
			fileDialog->SetCurrentPath(initialDir);
			fileDialog->SetFileDialogType(fileDialogType);
			fileDialog->SetOpen(true);
			fileDialog->m_output = "";
			fileDialog->SetInitialized(true);
			if (filter == "All")
				filter = "";
			fileDialog->m_filter = std::move(filter);
		}
	}

	bool Editor::UI::FileDialog::Initialized()
	{
		static FileDialog* fileDialog = EditorUIManager::GetInstance()->GetFileDialog();
		return fileDialog->m_initialized;
	}

}
