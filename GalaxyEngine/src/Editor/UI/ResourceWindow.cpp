#include "pch.h"
#include "Editor/UI/ResourceWindow.h"
#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"

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
			static Shared<Resource::IResource> rightClickedResource;
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
					rightClickedResource = resource.second;
				}

			}
			if (rightClickedResource)
			{
				ImGui::OpenPopup("ResourcePopup");
			}
			if (rightClickedResource && ImGui::BeginPopup("ResourcePopup"))
			{
				if (ImGui::MenuItem("Load"))
				{
					Resource::ResourceManager::GetOrLoad(rightClickedResource->GetFileInfo().GetFullPath());
				}
				ImGui::EndPopup();
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
}