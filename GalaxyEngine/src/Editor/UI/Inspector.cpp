#include "pch.h"
#include "Editor/UI/Inspector.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"

#include "Resource/Scene.h"

#include "Render/Camera.h"
#include "Render/EditorCamera.h"

#include "Editor/Gizmo.h"

#include "Wrapper/Reflection.h"

void Editor::UI::Inspector::Draw()
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

void Editor::UI::Inspector::ShowGameObject(Core::GameObject* object)
{
	if (!object->GetParent())
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
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("%llu", object->GetSceneGraphID());
	}
	ImGui::SameLine();
	ImGui::Text("%llu", object->GetUUID().operator uint64_t());
	ImGui::Separator();

	ImGui::BeginDisabled(!object->m_active);

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

		bool enable = object->m_components[i]->IsSelfEnable();
		if (ImGui::Checkbox("##", &enable))
		{
			object->m_components[i]->SetSelfEnable(enable);
		}
		ImGui::SameLine();

		bool destroy = true;
		bool open = ImGui::CollapsingHeader(object->m_components[i]->GetComponentName(), &destroy, ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("ID : %d", object->m_components[i]->GetIndex());
			ImGui::EndTooltip();
		}
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

	ImGui::EndDisabled();
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


void Editor::UI::Inspector::AddSelected(Weak<Core::GameObject> gameObject)
{
	auto it = std::remove_if(m_selectedGameObject.begin(), m_selectedGameObject.end(),
		[&](const Weak<Core::GameObject>& c) {	return c.lock() == gameObject.lock(); });
	if (it != m_selectedGameObject.end()) {
		m_selectedGameObject.erase(it);
		gameObject.lock()->m_selected = false;
	}
	else {
		if (m_selectedGameObject.empty())
		{
			// if empty, set gizmo object
			Shared<Editor::Gizmo> gizmo = gameObject.lock()->GetScene()->GetGizmo();
			gizmo->SetGameObject(gameObject);
		}
			
		m_selectedGameObject.push_back(gameObject);
		gameObject.lock()->m_selected = true;
	}
}

void Editor::UI::Inspector::SetSelected(Weak<Core::GameObject> gameObject)
{
	ClearSelected();
	AddSelected(gameObject);
}

void Editor::UI::Inspector::ClearSelected()
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

const List<Weak<Core::GameObject>>& Editor::UI::Inspector::GetSelected()
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

void Editor::UI::Inspector::RightClickPopup()
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
			Core::GameObject* owner = m_rightClicked.lock()->GetGameObject().get();
			uint32_t index = m_rightClicked.lock()->GetIndex();

			owner->ChangeComponentIndex(index, index - 1);

			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Move Down", buttonSize))
		{
			Core::GameObject* owner = m_rightClicked.lock()->GetGameObject().get();
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
