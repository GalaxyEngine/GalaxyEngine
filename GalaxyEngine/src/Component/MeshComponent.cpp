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
		static auto renderer = Wrapper::Renderer::GetInstance();
		if (!m_mesh.lock())
			return;
		m_mesh.lock()->Render(GetGameObject()->GetTransform()->GetModelMatrix(), m_materials, GetGameObject()->GetIndex());
	}

	void Component::MeshComponent::Serialize(Utils::Serializer& serializer)
	{
		serializer << Utils::PAIR::KEY << "Mesh" << Utils::PAIR::VALUE << (m_mesh.lock() ? m_mesh.lock()->GetFileInfo().GetRelativePath() : "");
		serializer << Utils::PAIR::KEY << "Material Count" << Utils::PAIR::VALUE << m_materials.size();

		serializer << Utils::PAIR::BEGIN_TAB;
		for (int i = 0; i < m_materials.size(); i++)
		{
			serializer << Utils::PAIR::KEY << ("Material " + std::to_string(i)) << Utils::PAIR::VALUE << (m_materials[i].lock() ? m_materials[i].lock()->GetFileInfo().GetRelativePath() : "");
		}
		serializer << Utils::PAIR::END_TAB;
	}

	void Component::MeshComponent::Deserialize(Utils::Parser& parser)
	{
		std::string meshPath = parser["Mesh"];
		m_mesh = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(meshPath);
		size_t materialCount = parser["Material Count"].As<int>();
		for (size_t i = 0; i < materialCount; i++)
		{
			std::string materialPath = parser["Material " + std::to_string(i)];
			Weak<Resource::Material> material = Resource::ResourceManager::GetOrLoad<Resource::Material>(materialPath);
			m_materials.push_back(material);
		}
	}

	void Component::MeshComponent::ShowInInspector()
	{
		Vec2f buttonSize = { ImGui::GetContentRegionAvail().x / 2.f, 0 };
		if (ImGui::Button(m_mesh.lock() ? m_mesh.lock()->GetFileInfo().GetFileName().c_str(): "Empty", buttonSize))
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
		static uint32_t selected = 0;
		static uint32_t clicked = 0;
		if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (uint32_t i = 0; i < m_materials.size(); i++)
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
			ImGui::PushID((int)i);
			m_materials[i].lock()->ShowInInspector();
			ImGui::PopID();
		}
	}

}