#include "pch.h"
#include "Editor/UI/Inspector.h"
#include "Editor/UI/FileExplorer.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"

#include "Resource/Scene.h"

#include "Render/Camera.h"
#include "Editor/EditorCamera.h"


#include "Editor/Gizmo.h"
#include "Editor/UI/EditorUIManager.h"

void Editor::UI::Inspector::Draw()
{
	if (!p_open)
		return;
	if (ImGui::Begin("Inspector"))
	{
		if (m_mode == InspectorMode::Scene && m_selectedGameObject.size() == 1)
		{
			if (!m_selectedGameObject[0].expired())
				ShowGameObject(m_selectedGameObject[0].lock().get());
		}
		else if (m_mode == InspectorMode::Asset && m_selectedFiles->size() == 1)
		{
			ShowFile((*m_selectedFiles)[0].get());
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
		const bool open = ImGui::CollapsingHeader(object->m_components[i]->GetComponentName(), &destroy, ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
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
				const uint32_t payloadData = *static_cast<uint32_t*>(payload->Data);
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
	if (const std::shared_ptr<Component::BaseComponent> component = Wrapper::GUI::ComponentPopup())
	{
		object->AddComponent(component);
	}
}

void Editor::UI::Inspector::ShowFile(const File* file)
{
	if (file->m_info.isDirectory())
		return;
	constexpr int iconSize = 64;
	const Vec2f iconSizeXY = Vec2f{ iconSize };
	const auto resource = file->m_resource.lock();

	Wrapper::GUI::TextureImage(file->m_icon.lock().get(), iconSizeXY);
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::TextUnformatted(file->m_info.GetFileName().c_str());
	if (resource) {
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("ID : %llu", resource->GetUUID().operator uint64_t());
		ImGui::TextUnformatted(resource->GetResourceName());
	}
	else
		ImGui::TextUnformatted("None");
	ImGui::EndGroup();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_None, 2.f);
	if (!resource || !resource->IsLoaded())
		return;

	resource->ShowInInspector();
}

void Editor::UI::Inspector::AddSelected(const Weak<Core::GameObject>& gameObject)
{
	m_mode = InspectorMode::Scene;
	auto it = std::ranges::remove_if(m_selectedGameObject,
		[&](const Weak<Core::GameObject>& c) {	return c.lock() == gameObject.lock(); }).begin();
	if (it != m_selectedGameObject.end()) {
		m_selectedGameObject.erase(it);
		gameObject.lock()->m_selected = false;
	}
	else {
		if (m_selectedGameObject.empty())
		{
			// if empty, set gizmo object
			const Shared<Editor::Gizmo> gizmo = gameObject.lock()->GetScene()->GetGizmo();
			gizmo->SetGameObject(gameObject);
		}

		m_selectedGameObject.push_back(gameObject);
		gameObject.lock()->m_selected = true;
	}
}

void Editor::UI::Inspector::SetSelected(const Weak<Core::GameObject>& gameObject)
{
	ClearSelected();
	AddSelected(gameObject);
}

void Editor::UI::Inspector::SetFileSelected(List<Shared<File>>* files)
{
	m_selectedFiles = files;
}

void Editor::UI::Inspector::UpdateFileSelected()
{
	if (m_selectedFiles->empty())
	{
		m_mode = InspectorMode::None;
	}
	else
	{
		m_mode = InspectorMode::Asset;
	}
}

void Editor::UI::Inspector::ClearSelected()
{
	for (const auto& selected : m_selectedGameObject)
	{
		if (const Shared<Core::GameObject> object = selected.lock())
			object->m_selected = false;
	}
	m_selectedGameObject.clear();

	EditorUIManager::GetInstance()->GetFileExplorer()->ClearSelected();

	const Shared<Gizmo> gizmo = Core::SceneHolder::GetCurrentScene()->GetGizmo();
	gizmo->SetGameObject({});

	m_mode = InspectorMode::None;
}

const List<Weak<Core::GameObject>>& Editor::UI::Inspector::GetSelectedGameObjects()
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
		const Vec2f buttonSize(ImGui::GetContentRegionAvail().x, 0);
		if (ImGui::Button("Destroy", buttonSize))
		{
			m_rightClicked.lock()->RemoveFromGameObject();
			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Move Up", buttonSize))
		{
			Core::GameObject* owner = m_rightClicked.lock()->GetGameObject();
			const uint32_t index = m_rightClicked.lock()->GetIndex();

			owner->ChangeComponentIndex(index, index - 1);

			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Move Down", buttonSize))
		{
			Core::GameObject* owner = m_rightClicked.lock()->GetGameObject();
			const uint32_t index = m_rightClicked.lock()->GetIndex();

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
