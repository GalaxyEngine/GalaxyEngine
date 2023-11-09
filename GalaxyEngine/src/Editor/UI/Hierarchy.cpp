#include "pch.h"
#include "Editor/UI/EditorUIManager.h"
#include "Editor/UI/Hierarchy.h"
#include "Editor/UI/Inspector.h"

#include "Core/SceneHolder.h"
#include "Core/GameObject.h"

#include "Resource/Scene.h"

#include "Editor/ActionManager.h"

using namespace Core;
void Editor::UI::Hierarchy::Draw()
{
	if (!m_inspector)
		m_inspector = Editor::UI::EditorUIManager::GetInstance()->GetInspector();
	if (!p_open)
		return;
	if (ImGui::Begin("Hierarchy"))
	{
		Weak<GameObject> root = SceneHolder::GetInstance()->GetCurrentScene()->GetRootGameObject();
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

void Editor::UI::Hierarchy::DisplayGameObject(Weak<GameObject> weakGO, uint32_t index /* = 0*/)
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
		Wrapper::GUI::InputText("##InputText", &name);
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
		// Multi Select
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			m_inspector->AddSelected(weakGO);
		// already selected and not in multi select -> rename
		else if (gameobject->m_selected && m_inspector->GetSelected().size() == 1)
		{
			SetRename(gameobject);
		}
		// clear all selected and set selected
		else
			m_inspector->SetSelected(weakGO);
	}

	// Right Click
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && gameobject->GetParent())
	{
		m_openRightClick = true;
		// Do not reselect when is already selected.
		if (!gameobject->m_selected) {
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
				m_inspector->AddSelected(weakGO);
			else
				m_inspector->SetSelected(weakGO);
		}
	}
	else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !gameobject->GetParent())
	{
		m_openRightClick = true;
		m_inspector->ClearSelected();
	}
	else if (!ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered())
	{
		// Clear when none is clicked
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			m_inspector->ClearSelected();
		// clear and open right click popup when no one is right clicked
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			m_openRightClick = true;
			m_inspector->ClearSelected();
		}
	}
	if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		for (Weak<Core::GameObject>& object : m_inspector->GetSelected())
		{
			if (!object.lock())
				continue;

			Resource::Scene* currentScene = Core::SceneHolder::GetInstance()->GetCurrentScene();
			object.lock()->Destroy();
		}
	}

	// === Drag And Drop === //
	if (gameobject->m_parent.lock() && ImGui::BeginDragDropSource()) {

		List<uint64_t> indices;
		if (gameobject->m_selected) {
			List<Weak<Core::GameObject>> selected = m_inspector->GetSelected();
			for (size_t i = 0; i < selected.size(); i++)
			{
				indices.push_back(selected[i].lock()->m_id);
			}
		}
		else {
			indices.push_back(gameobject->m_id);
		}

		ImGui::SetDragDropPayload("GAMEOBJECTS", indices.data(), indices.size() * sizeof(uint64_t));
		ImGui::Text("Size : %d", (int)indices.size());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS")) {
			// Check if the payload data type matches
			List<uint64_t> indices;
			if (payload->DataSize % sizeof(uint64_t) == 0)
			{
				uint64_t* payloadData = static_cast<uint64_t*>(payload->Data);
				uint64_t payloadSize = payload->DataSize / sizeof(uint64_t);
				indices.assign(payloadData, payloadData + payloadSize);
			}
			for (size_t i = 0; i < indices.size(); i++) {
				Weak<GameObject> payloadGameObject = SceneHolder::GetInstance()->GetCurrentScene()->GetWithIndex(indices[i]);
				if (payloadGameObject.lock() && payloadGameObject.lock()->m_parent.lock() && !gameobject->IsAParent(payloadGameObject.lock().get()))
				{
					payloadGameObject.lock()->SetParent(weakGO);
					gameobject->m_open = true;
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	// Do the same for all children
	if (gameobject->m_open)
	{
		for (auto&& child : gameobject->m_childs)
		{
			if (!child)
				continue;
			ImGui::TreePush(child->m_name.c_str());
			DisplayGameObject(child, index++);
			ImGui::TreePop();
		}
	}
	ImGui::PopID();
}

void CreateGameObject(const List<Weak<Core::GameObject>>& selected)
{
	Resource::Scene* currentScene = Core::SceneHolder::GetInstance()->GetCurrentScene();
	if (selected.empty())
	{
		auto createObject = [currentScene = currentScene]() {
			Weak<Core::GameObject> gameObject = currentScene->CreateObject();
			currentScene->GetRootGameObject().lock()->AddChild(gameObject.lock());
			currentScene->GetRootGameObject().lock()->SetHierarchyOpen(true);
			};

		createObject();
		Editor::Action action(
			createObject,
			[currentScene = currentScene]()
			{
				currentScene->RevertObject();
			});

		currentScene->GetActionManager()->AddAction(action);
	}
	else
	{
		auto createObjects = [selected = selected, currentScene = currentScene]() {
			for (size_t i = 0; i < selected.size(); i++)
			{
				Weak<Core::GameObject> gameObject = currentScene->CreateObject();
				if (selected[i].lock()) {
					selected[i].lock()->AddChild(gameObject.lock());
					selected[i].lock()->SetHierarchyOpen(true);
				}
			}
			};

		createObjects();
		Editor::Action action(
			createObjects,
			[number = selected.size(), currentScene = currentScene]()
			{
				currentScene->RevertObject(number);
			});
		currentScene->GetActionManager()->AddAction(action);

	}
}

void Editor::UI::Hierarchy::RightClickPopup()
{
	if (ImGui::BeginPopup("RightClick", ImGuiWindowFlags_NoDecoration))
	{
		auto selected = m_inspector->GetSelected();
		Vec2f buttonSize = Vec2f(ImGui::GetWindowContentRegionWidth(), 0);
		if (ImGui::Button("Create GameObject", buttonSize))
		{
			CreateGameObject(selected);
			ImGui::CloseCurrentPopup();
		}

		// === At least one selected === //
		ImGui::BeginDisabled(selected.empty());
		{
			// === if selected are simbling === //
			if (!selected.empty()) {
				ImGui::BeginDisabled(!selected[0].lock()->IsSibling(selected));
			}
			{
				if (ImGui::Button("Create Parent", buttonSize)) // "Create Parent" button, clickable if at least one item is selected
				{
					auto parent = Core::SceneHolder::GetInstance()->GetCurrentScene()->CreateObject();
					uint32_t childIndex = selected[0].lock()->GetParent()->GetChildIndex(selected[0].lock().get());
					selected[0].lock()->GetParent()->AddChild(parent.lock(), childIndex);

					for (size_t i = 0; i < selected.size(); i++)
					{
						selected[i].lock()->SetParent(parent);
					}
					parent.lock()->m_open = true;
					ImGui::CloseCurrentPopup();
				}
			}
			if (!selected.empty()) {
				ImGui::EndDisabled();
			}
			// === if only one selected === //
			ImGui::BeginDisabled(selected.size() > 1);
			{
				if (ImGui::Button("Rename", buttonSize))
				{
					SetRename(selected[0].lock().get());
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndDisabled();

			if (ImGui::Button("Delete", buttonSize))
			{
				for (size_t i = 0; i < selected.size(); i++)
				{
					selected[i].lock()->Destroy();
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::EndDisabled();
		ImGui::EndPopup();
	}
}

void Editor::UI::Hierarchy::SetRename(Core::GameObject* gameObject)
{
	m_renameObject = gameObject;
	m_openRename = true;
}
