#include "pch.h"
Debug::Log::~Log()
{
}

#ifdef WITH_EDITOR
#include "Editor/UI/EditorUIManager.h"

void Debug::Log::AddTextToConsole(const LogType type, const std::string& text)
{
	if (const auto EditorUIManager = Editor::UI::EditorUIManager::GetInstance())
		EditorUIManager->GetConsole()->AddText(type, text);
}
#endif
