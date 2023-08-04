#include "pch.h"
#include "Component/MeshComponent.h"
#include "Resource/ResourceManager.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Core/GameObject.h"

namespace GALAXY {

	void Component::MeshComponent::OnDraw()
	{
		if (!m_mesh.lock())
			return;
		m_mesh.lock()->Render(gameObject.lock()->Transform()->GetModelMatrix(), m_materials);
	}

	void Component::MeshComponent::SetMesh(const std::weak_ptr<Resource::Mesh>& mesh)
	{
		if (!mesh.lock())
			return;
		m_mesh = mesh;
	}

	void Component::MeshComponent::ShowInInspector()
	{
		Vec2f buttonSize = { ImGui::GetContentRegionAvail().x / 2.f, 0 };
		if (ImGui::Button(m_mesh.lock() ? m_mesh.lock()->GetName().c_str() : "Empty", buttonSize))
		{
			ImGui::OpenPopup("MeshPopup");
		}
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, Vec4f(0.8f, 0.15f, 0.1f, 1.f));
		if (ImGui::Button("Reset", buttonSize))
		{
			m_mesh.reset();
		}
		ImGui::PopStyleColor();
		if (auto mesh = Resource::ResourceManager::GetInstance()->ResourcePopup<Resource::Mesh>("MeshPopup"); mesh.lock())
		{
			m_mesh = mesh;
		}
		static size_t selected = 0;
		static size_t clicked = 0;
		if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (size_t i = 0; i < m_materials.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::BeginGroup();
				Vec2f size = Vec2f(0, 16);
				if (ImGui::Selectable(("Element " + std::to_string(i)).c_str(), selected == i, ImGuiSelectableFlags_AllowItemOverlap, size))
				{
					selected = i;
				}
				ImGui::SameLine();
				Vec2f buttonSize = { ImGui::GetContentRegionAvail().x, size.y };
				if (ImGui::Button(m_materials[i].lock() ? m_materials[i].lock()->GetName().c_str() : "Missing", buttonSize))
				{
					ImGui::OpenPopup("MaterialPopup");
					clicked = i;
				}
				ImGui::EndGroup();
				ImGui::PopID();
				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload("MATERIAL", &i, sizeof(size_t));
					ImGui::Text("Element %d", i);
					ImGui::EndDragDropSource();
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL")) {
						size_t index = *static_cast<uint32_t*>(payload->Data);
						auto mat = m_materials[index];
						m_materials.erase(m_materials.begin() + index);
						m_materials.insert(m_materials.begin() +  i, mat);
					}
					ImGui::EndDragDropTarget();
				}
			}
			ImGui::PushID(clicked);
			if (auto mat = Resource::ResourceManager::GetInstance()->ResourcePopup<Resource::Material>("MaterialPopup"); mat.lock())
			{
				m_materials[clicked] = mat;
			}
			ImGui::PopID();
			if (ImGui::SmallButton("+"))
			{
				m_materials.push_back(std::weak_ptr<Resource::Material>());
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("-"))
			{
				if (selected >= 0 && selected < m_materials.size())
					m_materials.erase(m_materials.begin() + selected);
			}
			ImGui::TreePop();
		}

		// Debug
		for (size_t i = 0; i < m_materials.size(); i++)
		{
			if (!m_materials[i].lock())
				continue;
			m_materials[i].lock()->ShowInInspector();
		}

		if (ImGui::Button("Print Datas"))
		{
			GameObject()->Transform()->GetLocalMatrix().Print();
		}
	}

}