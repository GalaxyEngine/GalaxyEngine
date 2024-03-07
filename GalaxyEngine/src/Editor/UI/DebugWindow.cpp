#include "pch.h"
#include "Editor/UI/DebugWindow.h"
namespace GALAXY 
{

	void Editor::UI::DebugWindow::Draw()
	{
		if (!p_open)
			return;
		if (ImGui::Begin("Debug"), &p_open)
		{
			ImGui::Text("Triangle draw count: %zu", m_triangleDrawCount);
			ResetTriangleDrawCount();
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
