#include "pch.h"
#include "Editor/UI/FileDialog.h"
namespace GALAXY 
{

	void Editor::UI::FileDialog::Draw()
	{
		if (!p_open)
			return;

		if (ImGui::Begin("File Dialog", &p_open))
		{
			auto& it = std::filesystem::directory_iterator(m_currentPath);
			for (auto& entry : it)
			{
				if (entry.is_directory())
				{
					if (ImGui::Selectable(entry.path().filename().string().c_str()))
					{
						m_currentPath = entry.path();
					}
				}
			}
		}
		ImGui::End();
	}

}
