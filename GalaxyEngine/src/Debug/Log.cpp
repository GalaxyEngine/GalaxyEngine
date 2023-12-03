#include "pch.h"

#include "Editor/UI/EditorUIManager.h"

Debug::Log::~Log()
{
}

void Debug::Log::AddTextToConsole(const LogType type, const std::string& text)
{
	if (const auto EditorUIManager = Editor::UI::EditorUIManager::GetInstance())
		EditorUIManager->GetConsole()->AddText(type, text);
}
