#include "pch.h"

#include "Editor/UI/FileDialog.h"
#include "Editor/UI/FileExplorer.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

#define FOLDER_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME"/icons/folder.png"
#define FILE_ICON_PATH ENGINE_RESOURCE_FOLDER_NAME"/icons/file.png"

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

	void Editor::UI::TmpFile::FindChildrens()
	{
		try {

			auto iterator = std::filesystem::directory_iterator(m_info.GetFullPath());
			for (auto entry : iterator)
			{
				Shared<TmpFile> child = std::make_shared<TmpFile>(entry.path());
				m_childrens.push_back(child);
			}
		}
		catch (const std::exception&)
		{
			PrintError("Failed to find childrens files of %s", m_info.GetFullPath().string().c_str());
		}
	}

	Editor::UI::TmpFile::~TmpFile()
	{
	}

#pragma endregion


	Editor::UI::FileDialog::FileDialog()
	{
		SetCurrentPath(std::filesystem::current_path());
	}

	Editor::UI::FileDialog::~FileDialog()
	{
	}

	void Editor::UI::FileDialog::Draw()
	{
		if (!p_open)
			return;

		if (ImGui::Begin("File Dialog", &p_open))
		{
			if (ImGui::Button("<|"))
			{
				Path parentPath = m_currentPath.parent_path();
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

			static ImGuiTextFilter filter;
			filter.Draw("Search");

			ImGui::BeginChild("Content Panel");
			const int iconSize = 128;
			const int textLength = 10;
			const int space = 15;
			int i = 0;
			for (int i = 0, j = 0; i < m_currentFile->m_childrens.size(); i++)
			{
				const auto& file = m_currentFile->m_childrens[i];

				if (!filter.PassFilter(file->m_info.GetFileName().c_str()))
					continue;

				ImGui::PushID((int)i);

				Vec2f cursorPos = ImGui::GetCursorPos();

				// Handle file selection logic
				if (ImGui::Selectable("##select", &file->m_selected, ImGuiSelectableFlags_SelectOnClick, Vec2f(iconSize))) {
					if (file->m_selected || !file->m_selected && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
					{
						m_currentFile->SetSelected(file);
					}
				}
				Vec2f selectedCursorPos = ImGui::GetCursorPos();

				// Handle double-click to open the file
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (file->m_info.isDirectory())
					{
						SetCurrentPath(file->m_info.GetFullPath());
						break;
					}
				}
				else if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(file->m_info.GetFileName().c_str());
				}

				// Positioning for the file icon and text
				ImGui::SetCursorPos(cursorPos + Vec2f(12, 0));
				ImGui::SetCursorPos(cursorPos + Vec2f(12, 0));

				ImGui::BeginGroup();
				Wrapper::GUI::TextureImage(file->m_icon.get(), Vec2f(iconSize - 24));

				// Truncate and display file name
				size_t length = file->m_info.GetFileName().length();
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
		ImGui::End();
	}

	void Editor::UI::FileDialog::SetCurrentPath(Path val)
	{
		m_currentPath = val;
		m_currentFile = std::make_shared<TmpFile>(val);
		m_currentFile->FindChildrens();
	}

}
