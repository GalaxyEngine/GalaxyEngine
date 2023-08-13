#include "pch.h"

#include "EditorUI/EditorUIManager.h"

Debug::Log::~Log()
{
}

void Debug::Log::AddTextToConsole(LogType type, const std::string& text)
{
	EditorUI::EditorUIManager::GetInstance()->GetConsole()->AddText(type, text);
}
