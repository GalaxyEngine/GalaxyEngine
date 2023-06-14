#include "pch.h"
#include "EditorUI/Inspector.h"
#include "Core/GameObject.h"

void EditorUI::Inspector::Draw()
{
	if (!p_open)
		return;
	if (ImGui::Begin("Inspector"))
	{

	}
	ImGui::End();
}

void EditorUI::Inspector::AddSelected(std::weak_ptr<Core::GameObject> gameObject)
{
	auto it = std::remove_if(m_selectedGameObject.begin(), m_selectedGameObject.end(),
		[&](const std::weak_ptr<Core::GameObject>& c) {	return c.lock() == gameObject.lock(); });
	if (it != m_selectedGameObject.end()) {
		m_selectedGameObject.erase(it);
		gameObject.lock()->m_selected = false;
	}
	else {
		m_selectedGameObject.push_back(gameObject);
		gameObject.lock()->m_selected = true;
	}
}

void EditorUI::Inspector::SetSelected(std::weak_ptr<Core::GameObject> gameObject)
{
	ClearSelected();
	AddSelected(gameObject);
}

void EditorUI::Inspector::ClearSelected()
{
	for (size_t i = 0; i < m_selectedGameObject.size(); i++)
	{
		if (auto object = m_selectedGameObject[i].lock())
			object->m_selected = false;
	}
	m_selectedGameObject.clear();
}

std::vector<std::weak_ptr<Core::GameObject>> EditorUI::Inspector::GetSelected()
{
	for (size_t i = 0; i < m_selectedGameObject.size(); i++)
	{
		if (m_selectedGameObject[i].expired())
		{
			m_selectedGameObject.erase(m_selectedGameObject.begin() + i);
			--i;
		}
	}
	return m_selectedGameObject;
}
