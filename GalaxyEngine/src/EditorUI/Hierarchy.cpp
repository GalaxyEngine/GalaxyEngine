#include "pch.h"
#include "EditorUI/Hierarchy.h"
#include "Core/SceneHolder.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"

using namespace Core;
void EditorUI::Hierarchy::Draw()
{
	if (!p_open)
		return;
	if (ImGui::Begin("Hierarchy"))
	{
		std::weak_ptr<GameObject> root = SceneHolder::GetInstance()->GetCurrentScene()->GetRootGameObject();
		DisplayGameObject(root);

		if (m_openRightClick)
		{
			ImGui::OpenPopup("RightClick");
			m_openRightClick = false;
		}
		RightClickPopup();
	}
	ImGui::End();
}

void EditorUI::Hierarchy::DisplayGameObject(std::weak_ptr<GameObject> weakGO, uint32_t index /* = 0*/)
{
	GameObject* gameobject = weakGO.lock().get();
	if (!gameobject)
		return;

	ImGui::PushID(index);
	// Display arrow button
	if (gameobject->m_childs.size() > 0) {
		if (!gameobject->m_open) {
			if (ImGui::ArrowButton("##right", ImGuiDir_Right))
			{
				gameobject->m_open = true;
			}
		}
		else if (gameobject->m_open) {

			if (ImGui::ArrowButton("##down", ImGuiDir_Down))
			{
				gameobject->m_open = false;
			}
		}
		ImGui::SameLine(0, 10);
	}
	else
	{
		ImGui::Button("#", Vec2f(20));
		ImGui::SameLine();
	}

	// Rename Field
	if (m_renameObject == gameobject)
	{
		static std::string name;
		name = m_renameObject->m_name;
		if (m_openRename)
		{
			ImGui::SetKeyboardFocusHere();
		}
		ImGui::InputText("##InputText", &name);
		if (m_renameObject && !m_openRename && !ImGui::IsItemActive())
		{
			m_renameObject->m_name = name;
			m_renameObject = nullptr;
		}
		m_openRename = false;
	}
	// Selectable Field
	else if (ImGui::Selectable(gameobject->m_name.c_str(), gameobject->m_selected, ImGuiSelectableFlags_SelectOnNav))
	{
		gameobject->m_selected = true;
		//TODO
	}
	// Right Click
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && gameobject->GetParent().lock())
	{
		m_openRightClick = true;
		m_rightClicked = gameobject;
	}
	else if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		m_openRightClick = true;
	}

	// Drag And Drop
	if (gameobject->m_parent.lock() && ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("GAMEOBJECT", &gameobject->m_id, sizeof(uint64_t));
		ImGui::Text(gameobject->m_name.c_str());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT")) {
			uint64_t PayloadIndex = *(uint64_t*)payload->Data;
			std::weak_ptr<GameObject> payloadGameObject = SceneHolder::GetInstance()->GetCurrentScene()->GetWithIndex(PayloadIndex);
			if (payloadGameObject.lock() && payloadGameObject.lock()->m_parent.lock() && !gameobject->IsAParent(payloadGameObject.lock().get()))
			{
				payloadGameObject.lock()->SetParent(weakGO);
				gameobject->m_open = true;
			}
		}
		ImGui::EndDragDropTarget();
	}

	// Do the same for all childs
	if (gameobject->m_open)
	{
		for (auto&& child : gameobject->m_childs)
		{
			if (!child.lock())
				continue;
			ImGui::TreePush(child.lock()->m_name.c_str());
			DisplayGameObject(child, index++);
			ImGui::TreePop();
		}
	}
	ImGui::PopID();
}

void EditorUI::Hierarchy::RightClickPopup()
{
	if (ImGui::BeginPopup("RightClick", ImGuiWindowFlags_NoDecoration))
	{
		Vec2f buttonSize = Vec2f(ImGui::GetWindowContentRegionWidth(), 0);
		if (ImGui::Button("Create GameObject", buttonSize))
		{
			auto gameObject = Core::SceneHolder::GetInstance()->GetCurrentScene()->CreateObject();
			if (m_rightClicked) {
				m_rightClicked->AddChild(gameObject);
				m_rightClicked->m_open = true;
			}
			else
			{
				Core::SceneHolder::GetInstance()->GetCurrentScene()->GetRootGameObject().lock()->AddChild(gameObject);
			}
			ImGui::CloseCurrentPopup();
		}			
		ImGui::BeginDisabled(!m_rightClicked);
		if (ImGui::Button("Create Parent", buttonSize))
		{
			auto parent = Core::SceneHolder::GetInstance()->GetCurrentScene()->CreateObject();

			uint32_t childIndex = m_rightClicked->GetParent().lock()->GetChildIndex(m_rightClicked);
			m_rightClicked->GetParent().lock()->AddChild(parent, childIndex);
			m_rightClicked->SetParent(parent);
			parent.lock()->m_open = true;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("Rename", buttonSize))
		{
			m_openRename = true;
			m_renameObject = m_rightClicked;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("Delete", buttonSize))
		{
			Core::SceneHolder::GetInstance()->GetCurrentScene()->RemoveObject(m_rightClicked);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::EndPopup();
	}
	else if (m_rightClicked)
	{
		m_rightClicked = nullptr;
	}
}
