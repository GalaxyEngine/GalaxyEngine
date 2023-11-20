#include "pch.h"
#include "Resource/Mesh.h"
#include "Wrapper/Renderer.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Model.h"
#include "Resource/Scene.h"

#include "Core/SceneHolder.h"

#include "Component/Transform.h"

namespace GALAXY {

	Resource::Mesh::Mesh(const Path& fullPath) : IResource(fullPath)
	{
		p_fileInfo.m_resourceType = GetResourceType();
		std::string fullPathString = p_fileInfo.m_fullPath.string();
		p_fileInfo.m_fileName = fullPathString.substr(fullPathString.find_last_of(':') + 1);
		m_modelPath = fullPathString.substr(0, fullPathString.find_last_of(':'));
	}

	void Resource::Mesh::Load()
	{
		p_shouldBeLoaded = true;
		Resource::ResourceManager::GetInstance()->GetOrLoad<Model>(m_modelPath);
		p_loaded = true;
	}

	void Resource::Mesh::Send()
	{
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();

		renderer->CreateVertexArray(m_vertexArrayIndex);
		renderer->BindVertexArray(m_vertexArrayIndex);

		renderer->CreateVertexBuffer(m_vertexBufferIndex, m_finalVertices.data(), m_finalVertices.size() * sizeof(float));

		renderer->CreateIndexBuffer(m_indexBufferIndex, m_indices.data()->Data(), sizeof(Vec3i) * m_indices.size());

		renderer->VertexAttribPointer(0, 3, 8 * sizeof(float), (void*)(0));
		renderer->VertexAttribPointer(1, 2, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		renderer->VertexAttribPointer(2, 3, 8 * sizeof(float), (void*)(5 * sizeof(float)));

		renderer->UnbindVertexArray();
		renderer->UnbindVertexBuffer();

		p_hasBeenSent = true;
		PrintLog("Sended resource %s", GetFileInfo().GetFullPath().string().c_str());
	}

	void Resource::Mesh::Render(const Mat4& modelMatrix, const std::vector<std::weak_ptr<Resource::Material>>& materials, uint64_t id /*= -1*/)
	{
		if (!HasBeenSent() || !IsLoaded())
			return;
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
		renderer->BindVertexArray(m_vertexArrayIndex);

		for (size_t i = 0; i < materials.size(); i++) {
			if (!materials[i].lock() || i >= m_subMeshes.size())
				continue;
			auto shader = materials[i].lock()->SendValues(id).lock();
			if (shader == nullptr)
				continue;

			Resource::Scene* currentScene = Core::SceneHolder::GetInstance()->GetCurrentScene();
			shader->SendMat4("Model", modelMatrix);
			shader->SendMat4("MVP", currentScene->GetVP() * modelMatrix);
			shader->SendVec3f("CamUp", currentScene->GetCameraUp());
			shader->SendVec3f("CamRight", currentScene->GetCameraRight());

			renderer->DrawArrays(m_subMeshes[i].startIndex, m_subMeshes[i].count);
		}
		renderer->UnbindVertexArray();
	}
	
	Path Resource::Mesh::CreateMeshPath(const Path& modelPath, const Path& fileName)
	{
		return modelPath.wstring() + L":" + fileName.wstring();
	}

	void Resource::Mesh::ComputeBoundingBox()
	{
		ASSERT(!m_positions.empty());
		for (const auto& vertex : m_positions) {
			m_boundingBox.min.x = std::min(m_boundingBox.min.x, vertex.x);
			m_boundingBox.min.y = std::min(m_boundingBox.min.y, vertex.y);
			m_boundingBox.min.z = std::min(m_boundingBox.min.z, vertex.z);

			m_boundingBox.max.x = std::max(m_boundingBox.max.x, vertex.x);
			m_boundingBox.max.y = std::max(m_boundingBox.max.y, vertex.y);
			m_boundingBox.max.z = std::max(m_boundingBox.max.z, vertex.z);
		}
		m_boundingBox.center = (m_boundingBox.min + m_boundingBox.max) / 2.0f;
	}

	void Resource::Mesh::DrawBoundingBox(Component::Transform* transform)
	{
		BoundingBox box = GetBoundingBox();
		auto instance = Wrapper::Renderer::GetInstance();

		instance->DrawWireCube(transform->GetWorldPosition() + box.center, Vec3f(
			(box.max.x - box.min.x) / 2.0f,
			(box.max.y - box.min.y) / 2.0f,
			(box.max.z - box.min.z) / 2.0f
		) * transform->GetWorldScale(), transform->GetWorldRotation(), Vec4f(1, 0, 0, 1), 5.f);
	}
}