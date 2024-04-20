#include "pch.h"
#include "Editor/UI/DebugWindow.h"

#include "Editor/UI/EditorUIManager.h"

namespace GALAXY 
{

	void Editor::UI::DebugWindow::Draw()
	{
		if (!p_open)
			return;
		if (ImGui::Begin("Debug", &p_open))
		{
			ImGui::Text("Triangle draw count: %zu", m_triangleDrawCount);
			ResetTriangleDrawCount();

			std::set<Core::UUID> loadingResources = EditorUIManager::GetInstance()->GetLoadingResources();
			std::string label = "Loading Resources : " + std::to_string(loadingResources.size());
			if (!loadingResources.empty())
			{
				if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					for (const auto& resourceUUID : loadingResources)
					{
						auto resource = Resource::ResourceManager::GetResource(resourceUUID);
						ASSERT(resource.lock());
						ImGui::Text("%s", resource.lock()->GetFileInfo().GetRelativePath().string().c_str());
					}
				}
			}
		}
		ImGui::End();
	}

	void Editor::UI::DebugWindow::AddTriangleDraw(size_t count)
	{
		m_triangleDrawCount += count;
	}

	void Editor::UI::DebugWindow::ResetTriangleDrawCount()
	{
		m_triangleDrawCount = 0;
	}

}
