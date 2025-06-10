#include "pch.h"
#include "Editor/UI/ResourceWindow.h"
#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Model.h"

using namespace Editor::UI;
namespace GALAXY
{
	void ResourceWindow::Initialize()
	{
		p_open = false;
		m_resources = Resource::ResourceManager::GetInstance()->GetAllResourcesPtr();
	}

	void ResourceWindow::Draw()
	{
		if (m_selectedResource && m_debugThumbnail)
		{
			if (auto selectedModel = std::dynamic_pointer_cast<Resource::Model>(m_selectedResource); selectedModel)
				selectedModel->CreateThumbnail();
			else if (auto selectedMaterial = std::dynamic_pointer_cast<Resource::Material>(m_selectedResource); selectedMaterial)
				selectedMaterial->CreateThumbnail();
		}
		if (!p_open)
			return;
		if (ImGui::Begin("Resources", &p_open))
		{
			int value = static_cast<int>(m_resourceDirDisplay) - 1;
			if (ImGui::Combo("Display Type", &value, SerializeResourceDirEnum()))
			{
				m_resourceDirDisplay = static_cast<ResourceDir>(value + 1);
			}
			ImGui::SameLine();
			static ImGuiTextFilter filter;
			filter.Draw();
			ImGui::BeginChild("List", Vec2f(0), true);
			for (auto& resource : *m_resources)
			{
				if (!filter.PassFilter(resource.first.string().c_str()))
					continue;
				if (m_resourceDirDisplay != ResourceDir::Both) {
					if (m_resourceDirDisplay != resource.second->GetFileInfo().GetResourceDir())
						continue;
				}
				if (ImGui::TreeNodeEx(resource.first.string().c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
				{
					ImGui::BeginDisabled(true);
					bool shouldBeLoaded = resource.second->ShouldBeLoaded();
					ImGui::Checkbox("Should be loaded", &shouldBeLoaded);
					bool loaded = resource.second->IsLoaded();
					ImGui::Checkbox("Is Loaded", &loaded);
					bool sent = resource.second->HasBeenSent();
					ImGui::Checkbox("Has Been Sent", &sent);
					ImGui::EndDisabled();
					ImGui::TreePop();
				}
				if (ImGui::IsItemClicked())
				{
					auto fileExplorer = Editor::UI::EditorUIManager::GetInstance()->GetFileExplorer();
					Shared<File> file = std::make_shared<File>(resource.second->GetFileInfo().GetFullPath());
					fileExplorer->ClearSelected();
					fileExplorer->AddFileSelected({ file });
				}
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					m_rightClickedResource = resource.second;
					m_shouldOpenPopup = true;
				}
			}
			if (m_shouldOpenPopup)
			{
				m_shouldOpenPopup = false;
				ImGui::OpenPopup("ResourcePopup");
			}
			if (m_rightClickedResource && ImGui::BeginPopup("ResourcePopup"))
			{
				bool shouldClosePopup = false;
				if (ImGui::MenuItem("Load"))
				{
					Resource::ResourceManager::GetOrLoad(m_rightClickedResource->GetFileInfo().GetFullPath());

					shouldClosePopup = true;
				}
				Resource::ResourceType resourceType = m_rightClickedResource->GetFileInfo().GetResourceType();
				if (resourceType == Resource::ResourceType::Model || (resourceType == Resource::ResourceType::Material
					&& ImGui::MenuItem(m_debugThumbnail ? "Stop Debug Thumbnail" : "Debug Thumbnail")))
				{
					m_debugThumbnail = !m_debugThumbnail;
					m_selectedResource = m_rightClickedResource;

					shouldClosePopup = true;
				}
				if (ImGui::MenuItem("Reload thumbnail"))
				{
					String uuidString = m_rightClickedResource->GetFileInfo().GetFileNameNoExtension();
					Core::UUID uuid = std::stoull(uuidString);
					auto resourceLink = Resource::ResourceManager::GetOrLoad(uuid).lock();
					if (resourceLink)
						resourceLink->CreateThumbnail();
					shouldClosePopup = true;
				}
				if (shouldClosePopup)
				{
					ImGui::CloseCurrentPopup();
					m_rightClickedResource.reset();
				}
				ImGui::EndPopup();
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
}