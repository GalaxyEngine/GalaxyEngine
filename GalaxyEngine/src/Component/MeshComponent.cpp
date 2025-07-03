#include "pch.h"
#include "Component/MeshComponent.h"
#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "Render/Camera.h"

#include "Component/CameraComponent.h"

#include "Core/GameObject.h"
#include "Editor/UI/IconManager.h"

#if WITH_EDITOR
#include "Editor/EditorCamera.h"
#endif

namespace GALAXY {
#ifdef WITH_EDITOR
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
	
	void Component::MeshComponent::ShowInInspector()
	{
		// ImGui::Checkbox("Draw bounding box", &m_drawBoundingBox);
		// ImGui::Checkbox("Draw model bounding box", &m_drawModelBoundingBox);
		Resource::ResourceManager::ResourceField(m_mesh, "Mesh");
		static uint32_t selected = -1;
		bool clearSelected = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen |  ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
			for (uint32_t i = 0; i < m_materials.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::Separator();
				bool wasSelected = i == selected;
				Vec2f cursorPos;
				Resource::ResourceManager::ResourceField(m_materials[i], "Element " + std::to_string(i), &wasSelected, &cursorPos);
				if (wasSelected && i != selected)
				{
					clearSelected = false;
				}
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
				auto prevCursorPos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(cursorPos);
				ImGui::SetCursorPos(cursorPos + Vec2f(ImGui::GetContentRegionAvail().x - ImGui::GetFrameHeight(), 0.f));
				if (i != 0 && m_materials.size() > 1 && ImGui::ArrowButton("Up", ImGuiDir_Up))
				{
					// Move up
					auto material = m_materials[i];
					m_materials.erase(m_materials.begin() + i);
					m_materials.insert(m_materials.begin() + i - 1, material);
				}
				ImGui::SetCursorPos(cursorPos);
				ImGui::SetCursorPos(cursorPos + Vec2f(ImGui::GetContentRegionAvail().x - ImGui::GetFrameHeight() * 2.f - ImGui::GetStyle().ItemSpacing.x, 0.f));
				if (i != m_materials.size() - 1 && m_materials.size() > 1 && ImGui::ArrowButton("Down", ImGuiDir_Down))
				{
					// Move down
					auto material = m_materials[i];
					m_materials.erase(m_materials.begin() + i);
					m_materials.insert(m_materials.begin() + i + 1, material);
				}
				ImGui::SetCursorPos(prevCursorPos);
				ImGui::PopID();
			}
			ImGui::Separator();

			Vec2f buttonSize = Vec2f(100.f, ImGui::GetFrameHeight() + 2.5f);
			if (UI::IconButton("Add", Editor::UI::IconManager::AddIcon, Vec2f(14), 4.f, buttonSize))
			{
				m_materials.push_back(std::weak_ptr<Resource::Material>());
			}
			if (selected >= 0 && selected < m_materials.size())
			{
				ImGui::SameLine();
				if (UI::IconButton("Remove", Editor::UI::IconManager::RemoveIcon, Vec2f(14), 4.f, buttonSize))
				{
					m_materials.erase(m_materials.begin() + selected);
				}
			}
			if (clearSelected)
				selected = -1;
		}
	}

	uint32_t Component::MeshComponent::GetIcon()
	{
		return Editor::UI::IconManager::MeshComponentIcon;
	}
#endif

	void Component::MeshComponent::OnDraw()
	{
		const auto gameObject = GetGameObject();
		
		auto mesh = m_mesh.lock();
		if (!mesh)
			return;
		
		const Shared<Render::Camera>& currentCamera = gameObject->GetScene()->GetCurrentCamera();
		bool testFrustum = Wrapper::Renderer::GetInstance()->GetRenderType() != Render::RenderType::Shadow;
		if (testFrustum && currentCamera && !mesh->GetBoundingBox().IsOnFrustum(currentCamera.get(), GetTransform()))
			return;
		
		mesh->Render(gameObject->GetTransform()->GetModelMatrix(), m_materials, gameObject->GetScene(), gameObject->GetSceneGraphID());
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

}