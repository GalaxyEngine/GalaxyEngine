#include "pch.h"
#include "EditorUI/Hierarchy.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"

using namespace Core;
void EditorUI::Hierarchy::Draw()
{
	if (!p_open)
		return;
	if (ImGui::Begin("Hierarchy"))
	{
		std::weak_ptr<GameObject> root = Scene::GetInstance()->GetRootGameObject();
		DisplayGameObject(root.lock().get());
	}
	ImGui::End();
}

void EditorUI::Hierarchy::DisplayGameObject(GameObject* gameobject, uint32_t index /* = 0*/)
{
	ImGui::PushID(index);
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
		ImGui::Button("#", Vec2f(19, 18));
		ImGui::SameLine();
	}
	if (ImGui::Selectable(gameobject->m_name.c_str(), gameobject->m_selected, ImGuiSelectableFlags_SelectOnNav))
	{
		gameobject->m_selected = true;
		//TODO
	}
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		gameobject->m_open = true;
	}

	// Drag And Drop
	/*
	if (gameobject->m_parent && WrapperUI::BeginDragDropSource()) {
		WrapperUI::SetDragDropPayload("GAMEOBJECT", &uuid, sizeof(uint64_t));
		WrapperUI::Text(this->m_name.c_str());
		WrapperUI::EndDragDropSource();
	}
	if (WrapperUI::BeginDragDropTarget()) {
		if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
			uint64_t PayloadIndex = *(uint64_t*)payload->Data;
			GameObject* gameObject = Core::App::Get().sceneManager->GetCurrentNode()->GetWithIndex(PayloadIndex);
			if (gameObject && gameObject->m_parent && !IsAParent(gameObject))
			{
				if (!isPrefab)
				{
					gameObject->SetParent(this);
				}
				m_open = true;
			}
		}
		WrapperUI::EndDragDropTarget();
	}
	*/

	if (gameobject->m_open)
	{
		for (auto&& child : gameobject->m_childs)
		{
			ImGui::TreePush(child->m_name.c_str());
			DisplayGameObject(child.get(), index++);
			ImGui::TreePop();
		}
	}
	ImGui::PopID();
}