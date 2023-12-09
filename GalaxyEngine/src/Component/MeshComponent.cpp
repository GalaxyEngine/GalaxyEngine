#include "pch.h"
#include "Component/MeshComponent.h"

#include "Resource/ResourceManager.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "Wrapper/Renderer.h"

#include "Core/GameObject.h"

#include "Utils/Parser.h"

namespace GALAXY {

	void Component::MeshComponent::OnDraw()
	{
		if (!m_mesh.lock())
			return;
		m_mesh.lock()->Render(GetGameObject()->GetTransform()->GetModelMatrix(), m_materials, GetGameObject()->GetSceneGraphID());

		if (m_drawBoundingBox)
			m_mesh.lock()->DrawBoundingBox(GetGameObject()->GetTransform());
	}

	void Component::MeshComponent::Serialize(Utils::Serializer& serializer)
	{
		serializer << Utils::Pair::KEY << "Mesh" << Utils::Pair::VALUE << (m_mesh.lock() ? m_mesh.lock()->GetUUID() : INDEX_NONE);
		serializer << Utils::Pair::KEY << "Material Count" << Utils::Pair::VALUE << m_materials.size();

		serializer << Utils::Pair::BEGIN_TAB;
		for (size_t i = 0; i < m_materials.size(); i++)
		{
			serializer << Utils::Pair::KEY << ("Material " + std::to_string(i)) << Utils::Pair::VALUE << (m_materials[i].lock() ? m_materials[i].lock()->GetUUID() : INDEX_NONE);
		}
		serializer << Utils::Pair::END_TAB;
	}

	void Component::MeshComponent::Deserialize(Utils::Parser& parser)
	{
		const uint64_t meshUUID = parser["Mesh"].As<uint64_t>();
		m_mesh = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(meshUUID);
		const size_t materialCount = parser["Material Count"].As<int>();
		for (size_t i = 0; i < materialCount; i++)
		{
			const uint64_t materialUUID = parser["Material " + std::to_string(i)].As<uint64_t>();
			Weak<Resource::Material> material = Resource::ResourceManager::GetOrLoad<Resource::Material>(materialUUID);
			m_materials.push_back(material);
		}
	}

	void Component::MeshComponent::ShowInInspector()
	{
		Vec2f buttonSize = { ImGui::GetContentRegionAvail().x, 0 };
		ImGui::Checkbox("Draw bounding box", &m_drawBoundingBox);
		if (ImGui::Button(m_mesh.lock() ? m_mesh.lock()->GetFileInfo().GetFileName().c_str(): "Empty", buttonSize))
		{
			ImGui::OpenPopup("MeshPopup");
		}
		Weak<Resource::Mesh> mesh;
		if (Resource::ResourceManager::GetInstance()->ResourcePopup("MeshPopup", mesh))
		{
			m_mesh = mesh;
		}
		static uint32_t selected = 0;
		static uint32_t clicked = 0;
		if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (uint32_t i = 0; i < m_materials.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::BeginGroup();
				Vec2f size = Vec2f(0, 16 * Wrapper::GUI::GetScaleFactor());
				if (ImGui::Selectable(("Element " + std::to_string(i)).c_str(), selected == i, ImGuiSelectableFlags_AllowItemOverlap, size))
				{
					selected = i;
				}
				ImGui::SameLine();
				buttonSize = { ImGui::GetContentRegionAvail().x, size.y };
				if (ImGui::Button(m_materials[i].lock() ? m_materials[i].lock()->GetFileInfo().GetFileName().c_str() : "Missing", buttonSize))
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
						const size_t index = *static_cast<uint32_t*>(payload->Data);
						auto mat = m_materials[index];
						m_materials.erase(m_materials.begin() + index);
						m_materials.insert(m_materials.begin() +  i, mat);
					}
					ImGui::EndDragDropTarget();
				}
			}
			ImGui::PushID(clicked);
			Weak<Resource::Material> mat;
			if (Resource::ResourceManager::GetInstance()->ResourcePopup("MaterialPopup", mat))
			{
				m_materials[clicked] = mat;
			}
			ImGui::PopID();
			ImGui::PushStyleColor(ImGuiCol_Button, Vec4f(0.15f, 0.8f, 0.1f, 1.f));
			if (ImGui::Button("Add"))
			{
				m_materials.push_back(std::weak_ptr<Resource::Material>());
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, Vec4f(0.8f, 0.15f, 0.1f, 1.f));
			if (ImGui::Button("Remove"))
			{
				if (selected >= 0 && selected < m_materials.size())
					m_materials.erase(m_materials.begin() + selected);
			}
			ImGui::PopStyleColor();
			ImGui::TreePop();
		}

		// Debug
		for (size_t i = 0; i < m_materials.size(); i++)
		{
			if (!m_materials[i].lock())
				continue;
			ImGui::PushID(static_cast<int>(i));
			m_materials[i].lock()->ShowInInspector();
			ImGui::PopID();
		}
	}

}