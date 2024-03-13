#include "pch.h"
#include "Resource/Mesh.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Model.h"
#include "Resource/Scene.h"

#include "Core/SceneHolder.h"

#include "Component/Transform.h"

#include "Wrapper/Renderer.h"

#include "Render/Camera.h"
namespace GALAXY {

	Resource::Mesh::Mesh(const Path& fullPath) : IResource(fullPath)
	{
		p_status = ResourceStatus::DisplayOnInspector;
		const std::string fullPathString = p_fileInfo.m_fullPath.string();
		p_fileInfo.m_resourceType = ResourceType::Mesh;
	}

	const char* Resource::Mesh::GetMeshName()
	{
		auto meshName = GetFileInfo().GetFileName();
		meshName = meshName.substr(meshName.find(':') + 1);
		return meshName.c_str();
	}

	void Resource::Mesh::Load()
	{
		if (p_shouldBeLoaded)
			return;
		ASSERT(HasModel());
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

		PrintLog("Sended resource %s", GetFileInfo().GetFullPath().string().c_str());

		OnLoad.Invoke();

		m_finalVertices.clear();
		m_finalVertices.shrink_to_fit();

		m_indices.clear();
		m_indices.shrink_to_fit();
	}

	void Resource::Mesh::Render(const Mat4& modelMatrix, const std::vector<Weak<Resource::Material>>& materials, uint64_t id /*= -1*/) const
	{
		Render(modelMatrix, materials, nullptr, id);
	}
	
	void Resource::Mesh::Render(const Mat4& modelMatrix, const std::vector<Weak<class Material>>& materials, Resource::Scene* scene, uint64_t id /*= -1*/) const
	{
		if (!HasBeenSent() || !IsLoaded())
			return;
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
		renderer->BindVertexArray(m_vertexArrayIndex);

		if (!scene)
			scene = Core::SceneHolder::GetCurrentScene();

		const Vec3f viewPos = scene->GetCurrentCamera()->GetTransform()->GetLocalPosition();

		for (size_t i = 0; i < materials.size(); i++) {
			if (!materials[i].lock() || i >= m_subMeshes.size())
				continue;
			auto shader = materials[i].lock()->SendValues(id).lock();
			if (shader == nullptr)
				continue;

			const Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
			shader->SendMat4("Model", modelMatrix);
			shader->SendMat4("MVP", scene->GetVP() * modelMatrix);
			shader->SendVec3f("ViewPos", viewPos);
			shader->SendVec3f("CamUp", scene->GetCameraUp());
			shader->SendVec3f("CamRight", scene->GetCameraRight());

			renderer->DrawArrays(m_subMeshes[i].startIndex, m_subMeshes[i].count);
		}
		renderer->UnbindVertexArray();
	}

	Path Resource::Mesh::CreateMeshPath(const Path& modelPath, const Path& fileName)
	{
		return modelPath.wstring() + L":" + fileName.wstring();
	}

	void Resource::Mesh::ComputeBoundingBox(const std::vector<Vec3f>& positionVertices)
	{
		ASSERT(!positionVertices.empty());
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