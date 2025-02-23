#include "pch.h"
#include "Editor/UI/EditorWindow.h"

void Editor::UI::EditorWindow::Draw()
{
    if (!p_open)
    {
        p_visible = false;
        return;
    }
    if (p_shouldFocus)
    {
        ImGui::SetNextWindowFocus();
        p_shouldFocus = false;
    }
}

bool Editor::UI::EditorWindow::Begin(const char* name)
{
    bool value = (p_visible = ImGui::Begin(name, &p_open));

    if (value)
    {
        p_isFocused = ImGui::IsWindowFocused();
        p_isHovered = ImGui::IsWindowHovered();
    }

    return value;
}
