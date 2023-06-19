#include "pch.h"
#include "Resource/Mesh.h"
#include "Wrapper/Renderer.h"
#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Core/SceneHolder.h"
#include "Core/Scene.h"

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
}

void Resource::Mesh::Render(const Mat4& modelMatrix)
{
	auto shader = Resource::ResourceManager::GetInstance()->GetDefaultShader().lock();
	if (!p_hasBeenSent || !shader || !shader->HasBeenSent())
		return;
	Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
	shader->Use();
	renderer->BindVertexArray(m_vertexArrayIndex);

	renderer->ShaderSendMat4(shader->GetLocation("MVP"), modelMatrix.GetTranspose() * Core::SceneHolder::GetInstance()->GetCurrentScene()->GetVP());
	renderer->DrawArrays(0, m_indices.size() * 3);
	renderer->UnbindVertexArray();
}
