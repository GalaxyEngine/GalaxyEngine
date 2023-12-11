#include "pch.h"
#include "Editor/UI/ResourceWindow.h"

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
		if (ImGui::Begin("Resources"), &p_open)
		{
			int value = static_cast<int>(m_resourceDirDisplay) - 1;
			if (ImGui::Combo("Display Type", &value, SerializeResourceDirEnum()))
			{
				m_resourceDirDisplay = static_cast<ResourceDir>(value + 1);
			}
			for (auto& resource : *m_resources)
			{
				if (m_resourceDirDisplay != ResourceDir::Both) {
					if (m_resourceDirDisplay != resource.second->GetFileInfo().GetResourceDir())
						continue;
				}
				if (ImGui::TreeNode(resource.first.string().c_str()))
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

			}
		}
		ImGui::End();
	}
}