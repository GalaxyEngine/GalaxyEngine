#include "pch.h"

#include "EditorUI/EditorUIManager.h"

Debug::Log::~Log()
{
}

void Debug::Log::AddTextToConsole(LogType type, const std::string& text)
{
	if (auto editorUIManager = EditorUI::EditorUIManager::GetInstance())
		editorUIManager->GetConsole()->AddText(type, text);
}
