#include "pch.h"
#include "Resource/Mesh.h"
#include "Wrapper/Renderer.h"
#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"

#include <glad/glad.h>

GLuint positionVBO;
GLuint normalVBO;
GLuint uvVBO;
GLuint vbo;
GLuint ebo;
GLuint vao;
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

void Resource::Mesh::Render()
{
	auto shader = Resource::ResourceManager::GetInstance()->GetDefaultShader();
	if (!p_hasBeenSent || !shader.lock() || !shader.lock()->HasBeenSent())
		return;
	Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
	shader.lock()->Use();

	renderer->BindVertexArray(m_vertexArrayIndex);
	renderer->DrawArrays(0, m_indices.size() * 3);
	renderer->UnbindVertexArray();
}
