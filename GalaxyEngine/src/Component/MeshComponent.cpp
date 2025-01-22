#include "pch.h"
#include "Component/MeshComponent.h"
#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "Render/Camera.h"

#include "Component/CameraComponent.h"

#include "Core/GameObject.h"

#if WITH_EDITOR
#include "Editor/EditorCamera.h"
#endif

namespace GALAXY {
	void Component::MeshComponent::OnEditorDraw()
	{
		const auto gameObject = GetGameObject();
		
		Shared<Resource::Mesh> mesh = m_mesh.lock();
		if (!mesh)
			return;
		
		if (m_drawBoundingBox)
			mesh->DrawBoundingBox(gameObject->GetTransform());

		if (m_drawModelBoundingBox)
			mesh->GetModel()->DrawBoundingBox(gameObject->GetTransform());
	}

	void Component::MeshComponent::OnDraw()
	{
		const auto gameObject = GetGameObject();
		
		auto mesh = m_mesh.lock();
		if (!mesh)
			return;
		
		const Shared<Render::Camera>& currentCamera = gameObject->GetScene()->GetCurrentCamera();
		if (currentCamera && !mesh->GetBoundingBox().IsOnFrustum(currentCamera.get(), GetTransform()))
			return;
		
		m_mesh.lock()->Render(gameObject->GetTransform()->GetModelMatrix(), m_materials, gameObject->GetScene(), gameObject->GetSceneGraphID());
	}

	void Component::MeshComponent::Serialize(CppSer::Serializer& serializer)
	{
		if (m_mesh.lock())
			serializer << CppSer::Pair::Key << "Model" << CppSer::Pair::Value << m_mesh.lock()->GetModel()->GetUUID();
		else
			serializer << CppSer::Pair::Key << "Model" << CppSer::Pair::Value << UUID_NULL;

		serializer << CppSer::Pair::Key << "Mesh Name" << CppSer::Pair::Value << (m_mesh.lock() ? m_mesh.lock()->GetMeshName() : NONE_RESOURCE);
		serializer << CppSer::Pair::Key << "Material Count" << CppSer::Pair::Value << m_materials.size();

		serializer << CppSer::Pair::BeginTab;
		for (size_t i = 0; i < m_materials.size(); i++)
		{
			Resource::IResource::SerializeResource(serializer, ("Material " + std::to_string(i)).c_str(), m_materials[i]);
		}
		serializer << CppSer::Pair::EndTab;
	}

	void Component::MeshComponent::Deserialize(CppSer::Parser& parser)
	{
		const uint64_t modelUUID = parser["Model"].As<uint64_t>();
		const std::string meshName = parser["Mesh Name"];
		const auto model = Resource::ResourceManager::GetOrLoad<Resource::Model>(modelUUID);

		if (model.lock())
		{
			auto meshPath = Resource::Mesh::CreateMeshPath(model.lock()->GetFileInfo().GetFullPath(), meshName);
			m_mesh = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(meshPath);
		}
		else
		{
			PrintError("Model with uuid %llu not found", modelUUID);
		}

		const size_t materialCount = parser["Material Count"].As<int>();
		for (size_t i = 0; i < materialCount; i++)
		{
			const uint64_t materialUUID = parser["Material " + std::to_string(i)].As<uint64_t>();
			Weak<Resource::Material> material = Resource::ResourceManager::GetOrLoad<Resource::Material>(materialUUID);
			m_materials.push_back(material);
		}
	}

	void Component::MeshComponent::AddMaterial(const Weak<Resource::Material>& material)
	{
		m_materials.push_back(material);
	}

	void Component::MeshComponent::RemoveMaterial(size_t index)
	{
		if (m_materials.size() > index)
		{
			m_materials.erase(m_materials.begin() + index);
		}
		else
		{
			PrintError("Material index out of range");
		}
	}

	void Component::MeshComponent::ClearMaterials()
	{
		m_materials.clear();
	}

	
	void Component::MeshComponent::ShowInInspector()
	{
#ifdef WITH_EDITOR
		Vec2f buttonSize = { ImGui::GetContentRegionAvail().x, 0 };
		ImGui::Checkbox("Draw bounding box", &m_drawBoundingBox);
		ImGui::Checkbox("Draw model bounding box", &m_drawModelBoundingBox);
		Resource::ResourceManager::ResourceField(m_mesh, "Mesh");
		static uint32_t selected = -1;
		static uint32_t clicked = 0;
		if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (uint32_t i = 0; i < m_materials.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::Separator();
				ImGui::BeginGroup();
				const Vec2f size = Vec2f(0, 64 * Wrapper::GUI::GetScaleFactor());
				const Vec2f prevPos = ImGui::GetCursorPos();
				if (i != 0 && m_materials.size() > 1 && ImGui::ArrowButton("Up", ImGuiDir_Up))
				{
					// Move up
					auto material = m_materials[i];
					m_materials.erase(m_materials.begin() + i);
					m_materials.insert(m_materials.begin() + i - 1, material);
				}
				else
				{
					ImGui::InvisibleButton("Up", Vec2f(0, ImGui::GetFrameHeight()));
				}
				ImGui::SetCursorPosY(prevPos.y + size.y - ImGui::GetFrameHeight());
				if (i != m_materials.size() - 1 && m_materials.size() > 1 && ImGui::ArrowButton("Down", ImGuiDir_Down))
				{
					// Move down
					auto material = m_materials[i];
					m_materials.erase(m_materials.begin() + i);
					m_materials.insert(m_materials.begin() + i + 1, material);
				}
				else
				{
					ImGui::InvisibleButton("Down", Vec2f(0, ImGui::GetFrameHeight()));
				}
				ImGui::EndGroup();
				ImGui::SameLine();
				bool wasSelected = i == selected;
				Resource::ResourceManager::ResourceField(m_materials[i], "Element " + std::to_string(i), &wasSelected);
				if (wasSelected != (i == selected))
				{
					selected = i;
				}
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
						m_materials.insert(m_materials.begin() + i, mat);
					}
					ImGui::EndDragDropTarget();
				}
			}
			ImGui::PushStyleColor(ImGuiCol_Button, Vec4f(0.15f, 0.8f, 0.1f, 1.f));
			if (ImGui::Button("Add"))
			{
				m_materials.push_back(std::weak_ptr<Resource::Material>());
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
			if (ImGui::Button("Remove"))
			{
				if (selected >= 0 && selected < m_materials.size())
					m_materials.erase(m_materials.begin() + selected);
			}
			ImGui::PopStyleColor();
			ImGui::TreePop();
		}
		
		// Debug
		if (ImGui::Button("Create Object"))
		{
			auto object = m_mesh.lock()->ToGameObject();
			object->SetName("New Object");
			p_gameObject->AddChild(object);
		}
		
		/*
		if (m_mesh.lock())
		{
			if (ImGui::Button("Print Material of Mesh"))
			{
				for (auto material : m_mesh.lock()->GetMaterials())
				{
					PrintLog(material.lock()->GetName().c_str());
				}
			}
		}
		
		
		for (size_t i = 0; i < m_materials.size(); i++)
		{
			if (!m_materials[i].lock())
				continue;
			ImGui::PushID(static_cast<int>(i));
			m_materials[i].lock()->ShowInInspector();
			ImGui::PopID();
		}
		*/
#endif
	}

}