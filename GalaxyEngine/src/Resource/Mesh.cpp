#include "pch.h"
#include "Resource/Mesh.h"
#include "Wrapper/Renderer.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Model.h"

#include "Core/SceneHolder.h"
#include "Core/Scene.h"
namespace GALAXY {

	Resource::Mesh::Mesh(const std::filesystem::path& fullPath) : IResource(fullPath)
	{
		p_fileInfo.m_resourceType = GetResourceType();
		std::wstring fullPathString = p_fileInfo.m_fullPath.wstring();
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

	void Resource::Mesh::Render(const Mat4& modelMatrix, const std::vector<std::weak_ptr<Resource::Material>>& materials)
	{
		if (!HasBeenSent() || !IsLoaded())
			return;
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
		renderer->BindVertexArray(m_vertexArrayIndex);

		for (size_t i = 0; i < materials.size(); i++) {
			if (!materials[i].lock() || !materials[i].lock()->GetShader().lock()/* || !materials[i].lock()->GetShader().lock()->IsLoaded()*/)
				continue;
			materials[i].lock()->SendValues();

			renderer->ShaderSendMat4(materials[i].lock()->GetShader().lock()->GetLocation("MVP"), Core::SceneHolder::GetInstance()->GetCurrentScene()->GetVP() * modelMatrix);
			renderer->DrawArrays(0, m_indices.size() * 3);
		}
		renderer->UnbindVertexArray();
	}

	std::filesystem::path Resource::Mesh::CreateMeshPath(const std::filesystem::path& modelPath, const std::filesystem::path& fileName)
	{
		return modelPath.wstring() + L":" + fileName.wstring();
	}

}