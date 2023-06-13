#include "pch.h"
#include "EditorUI/EditorUIManager.h"

std::unique_ptr<EditorUI::EditorUIManager> EditorUI::EditorUIManager::m_instance;

void EditorUI::EditorUIManager::Initalize()
{
	m_hierarchy = std::make_unique<Hierarchy>();
}

void EditorUI::EditorUIManager::DrawUI()
{
	m_hierarchy->Draw();
}

EditorUI::EditorUIManager* GALAXY::EditorUI::EditorUIManager::GetInstance()
{
	if (!m_instance)
		m_instance = std::make_unique<EditorUIManager>();
	return m_instance.get();
}
