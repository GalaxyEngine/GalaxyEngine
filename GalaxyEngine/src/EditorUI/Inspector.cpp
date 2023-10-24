#include "pch.h"
#include "EditorUI/Inspector.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"

#include "Resource/Scene.h"

#include "Render/Camera.h"
#include "Render/EditorCamera.h"

#include "Editor/Gizmo.h"

#include "Wrapper/Reflection.h"

void EditorUI::Inspector::Draw()
{
	if (!p_open)
		return;
	if (ImGui::Begin("Inspector"))
	{
		if (m_selectedGameObject.size() == 1)
		{
			if (!m_selectedGameObject[0].expired())
				ShowGameObject(m_selectedGameObject[0].lock().get());
		}
	}
	ImGui::End();
}

void EditorUI::Inspector::ShowGameObject(Core::GameObject* object)
{
	if (!object->GetParent().lock())
	{
		Render::Camera::GetEditorCamera()->GetTransform()->ShowInInspector();
		if (ImGui::Button("Print Camera Datas")) {
			Render::Camera::GetEditorCamera()->GetTransform()->GetLocalPosition().Print();
		}
		return;
	}

	//TODO: Add tag & layer
	ImGui::Checkbox("##", &object->m_active);
	ImGui::SameLine();
	Wrapper::GUI::InputText("##InputName", &object->m_name);
	ImGui::SameLine();
	ImGui::TextUnformatted(std::to_string(object->GetIndex()).c_str());
	ImGui::Separator();

	// Transform
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		object->m_transform->ShowInInspector();
	}

	// Other Components
	bool openPopup = false;
	for (uint32_t i = 0; i < object->m_components.size(); i++) {
		if (!object->m_components[i].get())
			continue;
		ImGui::PushID(i);

		bool enable = object->m_components[i]->IsEnable();
		if (ImGui::Checkbox("##", &enable))
		{
			object->m_components[i]->SetEnable(enable);
		}
		ImGui::SameLine();

		bool destroy = true;
		bool open = ImGui::CollapsingHeader(object->m_components[i]->GetComponentName(), &destroy, ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			openPopup = true;
			m_rightClicked = object->m_components[i];
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("COMPONENT", &i, sizeof(uint32_t));
			ImGui::TextUnformatted(object->m_components[i]->GetComponentName());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMPONENT")) 
			{
				uint32_t payloadData = *static_cast<uint32_t*>(payload->Data);
				object->ChangeComponentIndex(payloadData, i);
			}
		}
		// Content of the Collapsing Header
		if (open) {
			ImGui::BeginDisabled(!enable);
			ImGui::TreePush(object->m_components[i]->GetComponentName());
			ImGui::Text("ID : %d", object->m_components[i]->GetIndex());
			object->m_components[i]->ShowInInspector();
			ImGui::TreePop();
			ImGui::EndDisabled();
		}

		ImGui::NewLine();
		ImGui::Separator();
		if (!destroy) {
			object->m_components[i]->RemoveFromGameObject();
			i--;
		}
		ImGui::PopID();
	}
	if (openPopup)
	{
		ImGui::OpenPopup("RightClickPopup");
	}
	RightClickPopup();
	// Add Component Button
	ImGui::NewLine();
	ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2 - 100);
	if (ImGui::Button("Add Component", Math::Vec2f(200, 0)))
	{
		ImGui::OpenPopup("ComponentPopup");
	}
	if (std::shared_ptr<Component::BaseComponent> component = Wrapper::GUI::ComponentPopup())
	{
		object->AddComponent(component);
	}
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

	Shared<Editor::Gizmo> gizmo = gameObject.lock()->GetScene()->GetGizmo();
	gizmo->SetGameObject(gameObject);
}

void EditorUI::Inspector::ClearSelected()
{
	for (size_t i = 0; i < m_selectedGameObject.size(); i++)
	{
		if (auto object = m_selectedGameObject[i].lock())
			object->m_selected = false;
	}
	m_selectedGameObject.clear();

	Shared<Editor::Gizmo> gizmo = Core::SceneHolder::GetCurrentScene()->GetGizmo();
	gizmo->SetGameObject({});
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

void EditorUI::Inspector::RightClickPopup()
{
	if (m_rightClicked.lock() && ImGui::BeginPopup("RightClickPopup"))
	{
		Vec2f buttonSize(ImGui::GetWindowContentRegionWidth(), 0);
		if (ImGui::Button("Destroy", buttonSize))
		{
			m_rightClicked.lock()->RemoveFromGameObject();
			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Move Up", buttonSize))
		{
			Core::GameObject* owner = m_rightClicked.lock()->gameObject.lock().get();
			uint32_t index = m_rightClicked.lock()->GetIndex();

			owner->ChangeComponentIndex(index, index - 1);

			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Move Down", buttonSize))
		{
			Core::GameObject* owner = m_rightClicked.lock()->gameObject.lock().get();
			uint32_t index = m_rightClicked.lock()->GetIndex();

			owner->ChangeComponentIndex(index, index + 1);

			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Reset", buttonSize))
		{
			m_rightClicked.lock()->Reset();
		}
		ImGui::EndPopup();
	}
}
