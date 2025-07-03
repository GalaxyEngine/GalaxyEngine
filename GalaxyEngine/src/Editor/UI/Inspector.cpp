#include "pch.h"
#include "Editor/UI/Inspector.h"

#include "Component/ScriptComponent.h"
#include "Core/Application.h"
#include "Editor/UI/FileExplorer.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"

#include "Resource/Scene.h"

#include "Render/Camera.h"
#include "Editor/EditorCamera.h"


#include "Editor/Gizmo.h"
#include "Editor/UI/Manager.h"
#include "Resource/Script.h"
#include "Utils/OS.h"

void Editor::UI::Inspector::Draw()
{
	EditorWindow::Draw();
	if (!p_open)
		return;
	if (Begin("Inspector"))
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

static bool CollapsingHeader(const char* label, uint32_t icon, bool* checked = nullptr, bool* destroyed = nullptr, bool* hovered = nullptr)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen
							 | ImGuiTreeNodeFlags_SpanAvailWidth
							 | ImGuiTreeNodeFlags_AllowItemOverlap
							 | ImGuiTreeNodeFlags_CollapsingHeader
							 | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	ImGui::PushID(label);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 5));
	bool is_open = ImGui::CollapsingHeader("##arrow", destroyed, flags);
	ImGui::PopStyleVar();

	if (hovered)
	{
		*hovered = ImGui::IsItemHovered();
	}
	
	ImGui::SameLine();

	Vec2f cursorPos = ImGui::GetCursorScreenPos();
	cursorPos.x = ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x + ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x;

	float size = ImGui::GetFrameHeight() * 1.f;

	cursorPos.y += ImGui::GetFrameHeight() * 0.1f;

	ImGui::GetWindowDrawList()->AddImage(icon, cursorPos, cursorPos + Vec2f(size), ImVec2(0, 0), ImVec2(1, 1));
	// ImGui::GetWindowDrawList()->AddRectFilled(cursorPos, cursorPos + Vec2f(size, size), IM_COL32(255, 0, 0, 255));
	
	ImGui::SetCursorScreenPos(cursorPos + Vec2f(size + ImGui::GetStyle().FramePadding.x * 2.f, 0));
	if (checked)
	{
		ImGui::Checkbox("##checked", checked);
	}
	else
	{
		ImGui::InvisibleButton("##invisible", Vec2f(ImGui::GetFrameHeight()));
	}
	
	ImGui::SameLine();
	
	ImGui::TextUnformatted(label);

	ImGui::PopID();
	return is_open;
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
	Wrapper::UI::InputText("##InputName", &object->m_name);
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("%llu", object->GetSceneGraphID());
	}
	ImGui::SameLine();
	ImGui::Text("%llu", object->GetUUID().operator uint64_t());
	ImGui::Separator();

	ImGui::BeginDisabled(!object->m_active);

	// Transform
	if (CollapsingHeader("Transform", object->m_transform->GetIcon(), nullptr, nullptr))
	{
		object->m_transform->ShowInInspector();
	}

	// Other Components
	bool openPopup = false;
	for (uint32_t i = 0; i < object->m_components.size(); i++) {
		if (!object->m_components[i])
			continue;
		ImGui::PushID(i);

		bool enable = object->m_components[i]->IsSelfEnable();
		bool destroy = true;
		bool hovered = false;
		const bool open = CollapsingHeader(object->m_components[i]->GetComponentName(), object->m_components[i]->GetIcon(), &enable, &destroy, &hovered);
		if (hovered)
		{
			ImGui::BeginTooltip();
			ImGui::Text("ID : %d", object->m_components[i]->GetIndex());
			ImGui::EndTooltip();
		}
		if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			openPopup = true;
			m_rightClicked = object->m_components[i];
		}
		if (enable != object->m_components[i]->IsSelfEnable())
		{
			object->m_components[i]->SetSelfEnable(enable);
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
			// ImGui::TreePush(object->m_components[i]->GetComponentName());
			object->m_components[i]->ShowInInspector();
			// ImGui::TreePop();
			ImGui::EndDisabled();
		}

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
	ImGui::SeparatorEx(ImGuiSeparatorFlags_SpanAllColumns, 2.f);
	ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2 - 100);
	if (Wrapper::UI::IconButton("Add Component", IconManager::AddIcon, Vec2f(16, 16), 4.0f, Vec2f(201.f, ImGui::GetFrameHeight() * 1.2f)))
	{
		ImGui::OpenPopup("ComponentPopup");
	}
	if (const std::shared_ptr<Component::BaseComponent> component = Wrapper::UI::ComponentPopup())
	{
		object->AddComponent(component);
	}
}

void Editor::UI::Inspector::ShowFile(const File* file) const
{
	if (file->m_info.isDirectory())
		return;
	constexpr int iconSize = 64;
	constexpr Vec2f iconSizeXY = Vec2f{ iconSize };
	const auto resource = file->m_resource.lock();

	Wrapper::UI::TextureImage(file->m_icon.lock().get(), iconSizeXY);
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
	if (m_selectedFiles->empty() && m_selectedGameObject.empty())
	{
		m_mode = InspectorMode::None;
	}
	else if (m_selectedFiles->empty())
	{
		m_mode = InspectorMode::Scene;
	}
	else
	{
		m_mode = InspectorMode::Asset;
		ClearSelectedGameObjects();
	}
}

void Editor::UI::Inspector::ClearSelectedGameObjects()
{
	for (const auto& selected : m_selectedGameObject)
	{
		if (const Shared<Core::GameObject> object = selected.lock())
			object->m_selected = false;
	}
	m_selectedGameObject.clear();

	const Shared<Gizmo> gizmo = Core::SceneHolder::GetCurrentScene()->GetGizmo();
	if (gizmo)
		gizmo->SetGameObject({});
}

void Editor::UI::Inspector::ClearSelected()
{
	ClearSelectedGameObjects();

	Manager::GetInstance()->GetFileExplorer()->ClearSelected();

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
		Shared<Component::BaseComponent> rightClicked = m_rightClicked.lock();
		const Vec2f buttonSize(ImGui::GetContentRegionAvail().x, 0);
		if (auto scriptComponent = std::dynamic_pointer_cast<Component::ScriptComponent>(rightClicked))
		{
			if (ImGui::Button("Edit", buttonSize))
			{
				auto filePath = Scripting::ScriptEngine::GetFilePathForScript(scriptComponent->GetComponentName());
				if (!filePath.empty())
				{
					Resource::Script::OpenScript(filePath);
					m_rightClicked.reset();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::Separator();
		}
		if (ImGui::Button("Destroy", buttonSize))
		{
			rightClicked->RemoveFromGameObject();
			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Move Up", buttonSize))
		{
			Core::GameObject* owner = rightClicked->GetGameObject();
			const uint32_t index = rightClicked->GetIndex();

			owner->ChangeComponentIndex(index, index - 1);

			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Move Down", buttonSize))
		{
			Core::GameObject* owner = rightClicked->GetGameObject();
			const uint32_t index = rightClicked->GetIndex();

			owner->ChangeComponentIndex(index, index + 1);

			m_rightClicked.reset();
			ImGui::CloseCurrentPopup();
		}
		else if (ImGui::Button("Reset", buttonSize))
		{
			rightClicked->Reset();
		}
		ImGui::EndPopup();
	}
}
