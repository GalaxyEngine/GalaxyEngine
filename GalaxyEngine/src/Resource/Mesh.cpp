#include "pch.h"
#include "Resource/Mesh.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Model.h"
#include "Resource/Scene.h"

#include "Core/SceneHolder.h"

#include "Component/Transform.h"

#ifdef WITH_EDITOR
#include "Editor/ThumbnailCreator.h"
#endif

#include "Component/Light.h"
#include "Component/MeshComponent.h"
#include "Core/Application.h"
#include "Wrapper/Renderer.h"

#include "Render/Camera.h"
#include "Render/Command.h"
#include "Render/LightManager.h"

namespace GALAXY {

	Resource::Mesh::Mesh(const Path& fullPath) : IResource(fullPath)
	{
		p_status = ResourceStatus::DisplayOnInspector;
		p_fileInfo.m_resourceType = ResourceType::Mesh;
	}

#ifdef WITH_EDITOR
	Path Resource::Mesh::GetThumbnailPath() const
	{
		return Editor::ThumbnailCreator::GetThumbnailPath(this);
	}

	void Resource::Mesh::CreateThumbnail()
	{
		Editor::ThumbnailCreator* thumbnailCreator = Core::Application::GetInstance().GetThumbnailCreator();
		thumbnailCreator->AddToQueue(shared_from_this());
	}
#endif
	
	void Resource::Mesh::SetMeshPosition(const std::vector<Vec3f>& positions)
	{
		if (p_loaded || p_hasBeenSent)
			return;
		m_positions = positions;
		p_loaded = true;
		p_shouldBeLoaded = true;
		for (size_t i = 0; i < positions.size(); i++)
		{
			m_finalVertices.push_back(positions[i].x);
			m_finalVertices.push_back(positions[i].y);
			m_finalVertices.push_back(positions[i].z);

			m_finalVertices.push_back(0);
			m_finalVertices.push_back(0);
			
			m_finalVertices.push_back(0);
			m_finalVertices.push_back(0);
			m_finalVertices.push_back(0);
			
			m_finalVertices.push_back(0);
			m_finalVertices.push_back(0);
			m_finalVertices.push_back(0);
		}
		SubMesh subMesh;
		subMesh.startIndex = 0;
		subMesh.count = positions.size();
		m_subMeshes.push_back(subMesh);
		ComputeBoundingBox(positions);
		Send();
	}

	std::string Resource::Mesh::GetMeshName()
	{
		auto meshName = GetFileInfo().GetFileName();
		meshName = meshName.substr(meshName.find(':') + 1);
		return meshName;
	}

	void Resource::Mesh::Load()
	{
		if (p_shouldBeLoaded)
			return;
		ASSERT(HasModel() && "Model for mesh not found");
		p_shouldBeLoaded = true;

		const std::string fullPathString = GetFileInfo().GetFullPath().string();
		const std::string modelPath = fullPathString.substr(0, fullPathString.find_last_of(':'));
		Resource::ResourceManager::GetOrLoad<Model>(modelPath);
		
		p_loaded = true;
	}

	void Resource::Mesh::Send()
	{
		if (p_hasBeenSent)
			return;
		p_hasBeenSent = true;
		StartLoading();
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();

		renderer->CreateVertexArray(m_vertexArrayIndex);
		renderer->BindVertexArray(m_vertexArrayIndex);

		renderer->CreateVertexBuffer(m_vertexBufferIndex, m_finalVertices.data(), m_finalVertices.size() * sizeof(float));

		//renderer->CreateIndexBuffer(m_indexBufferIndex, m_indices.data()->Data(), sizeof(Vec3i) * m_indices.size());

		constexpr int vertexSize = 11 * sizeof(float);
		const auto textureOffset = reinterpret_cast<void*>(3 * sizeof(float));
		const auto normalsOffset = reinterpret_cast<void*>(5 * sizeof(float));
		const auto tangentsOffset = reinterpret_cast<void*>(8 * sizeof(float));

		renderer->VertexAttribPointer(0, 3, vertexSize, nullptr);
		renderer->VertexAttribPointer(1, 2, vertexSize, textureOffset);
		renderer->VertexAttribPointer(2, 3, vertexSize, normalsOffset);
		renderer->VertexAttribPointer(3, 3, vertexSize, tangentsOffset);

		renderer->UnbindVertexArray();
		renderer->UnbindVertexBuffer();

		OnLoad.Invoke();

		m_finalVertices.clear();
		m_finalVertices.shrink_to_fit();

		m_indices.clear();
		m_indices.shrink_to_fit();
		FinishLoading();
	}

	void Resource::Mesh::Render(const Mat4& modelMatrix, const std::vector<Weak<Resource::Material>>& materials, uint64_t id /*= -1*/) const
	{
		Render(modelMatrix, materials, nullptr, id);
	}
	
	void Resource::Mesh::Render(const Mat4& modelMatrix, const std::vector<Weak<class Material>>& materials, Resource::Scene* scene, uint64_t id /*= -1*/) const
	{
		if (!HasBeenSent() || !IsLoaded())
			return;

		if (!scene)
			scene = Core::SceneHolder::GetCurrentScene();

		const Vec3f viewPos = scene->GetCurrentCamera() ? scene->GetCurrentCamera()->GetTransform()->GetWorldPosition() : Vec3f::Zero();

		Render::RenderType renderType = Wrapper::Renderer::GetInstance()->GetRenderType();

		const Mat4 MVP = scene->GetVP() * modelMatrix;
		for (size_t i = 0; i < materials.size(); i++) {
			Shared<Material> material = materials[i].lock();
			if (!material || i >= m_subMeshes.size())
				continue;

			Render::DrawCommandData data;
			
			Render::SortKey& sortKey = data.sortKey;
			sortKey.materialKey = material->GetUUID();
			sortKey.meshKey = GetUUID();
			
			data.vertexArrayID = m_vertexArrayIndex;
			data.material = material.get();
			data.subMesh = m_subMeshes[i];
			data.modelMatrix = modelMatrix;
			data.MVP = MVP;
			if (Shared<Component::Light> currLight = scene->GetCurrentLight())
			{
				data.hasLSM = true;
				data.LSM = currLight->GetViewProjectionMatrix();
			}
			data.ViewPos = viewPos;
			data.CamUp = scene->GetCameraUp();
			data.CamRight = scene->GetCameraRight();
			data.sceneID = id;

			switch (renderType)
			{
			case Render::RenderType::None:
				break;
			case Render::RenderType::Default:
				Render::CommandBuffer::AddCommand(std::make_unique<Render::DrawCommand>(data));
				break;
			case Render::RenderType::Picking:
				Render::CommandBuffer::AddCommand(std::make_unique<Render::DrawPickingCommand>(data));
				break;
			case Render::RenderType::Outline:
				Render::CommandBuffer::AddCommand(std::make_unique<Render::DrawOutlineCommand>(data));
				break;
			case Render::RenderType::PostProcess:
				Render::CommandBuffer::AddCommand(std::make_unique<Render::DrawPostProcessCommand>(data));
				break;
			case Render::RenderType::Shadow:
				Render::CommandBuffer::AddCommand(std::make_unique<Render::DrawShadowCommand>(data));
				break;
			default: ;
			}
		}
	}

	Path Resource::Mesh::CreateMeshPath(const Path& modelPath, const Path& fileName)
	{
		return modelPath.wstring() + L":" + fileName.wstring();
	}

	Shared<Core::GameObject> Resource::Mesh::ToGameObject()
	{
		Shared<Core::GameObject> meshGO = std::make_shared<Core::GameObject>(GetMeshName());
		auto meshComponent = meshGO->AddComponent<Component::MeshComponent>();
		auto self = std::dynamic_pointer_cast<Resource::Mesh>(shared_from_this());
		meshComponent.lock()->SetMesh(self);
		auto materials = GetMaterials();
		size_t index = 0;
		for (auto& subMesh : m_subMeshes) {
			UNUSED(subMesh);
			if (index < materials.size())
				meshComponent.lock()->AddMaterial(materials[index]);
			else
				meshComponent.lock()->AddMaterial(ResourceManager::GetInstance()->GetDefaultMaterial());
			index++;
		}
		return meshGO;
	}

	List<Weak<Resource::Material>> Resource::Mesh::GetMaterials() const
	{
		if (!m_model)
			return List<Weak<Resource::Material>>();
		return m_model->GetMaterialsOfMesh(this);
	}

	void Resource::Mesh::ComputeBoundingBox(const std::vector<Vec3f>& positionVertices)
	{
		// ASSERT(!positionVertices.empty());
		for (const auto& vertex : positionVertices) {
			m_boundingBox.min.x = std::min(m_boundingBox.min.x, vertex.x);
			m_boundingBox.min.y = std::min(m_boundingBox.min.y, vertex.y);
			m_boundingBox.min.z = std::min(m_boundingBox.min.z, vertex.z);

			m_boundingBox.max.x = std::max(m_boundingBox.max.x, vertex.x);
			m_boundingBox.max.y = std::max(m_boundingBox.max.y, vertex.y);
			m_boundingBox.max.z = std::max(m_boundingBox.max.z, vertex.z);
		}
	}

	void Resource::Mesh::DrawBoundingBox(const Component::Transform* transform) const
	{
		const BoundingBox box = GetBoundingBox();
		const auto instance = Wrapper::Renderer::GetInstance();

		instance->DrawWireCube(transform->GetWorldPosition() + box.GetCenter(), Vec3f(
			(box.max.x - box.min.x) / 2.0f,
			(box.max.y - box.min.y) / 2.0f,
			(box.max.z - box.min.z) / 2.0f
		) * transform->GetWorldScale(), Vec4f(1, 0, 0, 1), 5.f);
	}
}
