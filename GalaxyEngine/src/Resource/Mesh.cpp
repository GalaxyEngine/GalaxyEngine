#include "pch.h"
#include "Resource/Mesh.h"
#include "Wrapper/Renderer.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Model.h"

#include "Core/SceneHolder.h"
#include "Resource/Scene.h"
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
			if (!materials[i].lock())
				continue;
			auto shader = materials[i].lock()->SendValues(id);
			if (shader.lock() == nullptr)
				continue;

			shader.lock()->SendMat4("MVP", Core::SceneHolder::GetInstance()->GetCurrentScene()->GetVP() * modelMatrix);
			renderer->DrawArrays(0, m_indices.size() * 3);
		}
		renderer->UnbindVertexArray();
	}

	Path Resource::Mesh::CreateMeshPath(const Path& modelPath, const Path& fileName)
	{
		return modelPath.wstring() + L":" + fileName.wstring();
	}

}