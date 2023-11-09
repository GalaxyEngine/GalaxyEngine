#include "pch.h"

#include "Editor/UI/EditorUIManager.h"

Debug::Log::~Log()
{
}

void Debug::Log::AddTextToConsole(LogType type, const std::string& text)
{
	if (auto EditorUIManager = Editor::UI::EditorUIManager::GetInstance())
		EditorUIManager->GetConsole()->AddText(type, text);
}
