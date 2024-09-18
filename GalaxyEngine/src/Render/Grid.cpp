#include "pch.h"
#include "Render/Grid.h"
#include "Render/Camera.h"

#include "Resource/ResourceManager.h"
#include "Resource/ResourceManager.h"

#include "Core/SceneHolder.h"

#include <glad/glad.h>

namespace GALAXY
{

	Render::Grid::~Grid()
	{
		if (m_vbo != 0) {
			glDeleteBuffers(1, &m_vbo);
			m_vbo = 0;
		}
		if (m_vao != 0) {
			glDeleteVertexArrays(1, &m_vao);
			m_vao = 0;
		}
	}

	void Render::Grid::Initialize()
	{
		m_shader = Resource::ResourceManager::GetOrLoad<Resource::Shader>(GRID_PATH);

		Wrapper::Renderer::GetInstance()->CreateDynamicVertexBuffer(m_vao, m_vbo, 6 * 4 * sizeof(float), 4);
	}

	void Render::Grid::Draw() const
	{
		if (const Shared<Resource::Shader> gridShader = m_shader.lock(); gridShader && gridShader->HasBeenSent()) {

			const auto renderer = Wrapper::Renderer::GetInstance();
			const auto currentCamera = Render::Camera::GetCurrentCamera();
			const auto scene = Core::SceneHolder::GetCurrentScene();

			renderer->BindVertexArray(m_vao);

			gridShader->Use();

			gridShader->SendMat4("view", scene->GetView());
			gridShader->SendMat4("proj", scene->GetProjection());
			gridShader->SendFloat("near", currentCamera->GetNear());
			gridShader->SendFloat("far", currentCamera->GetFar());

			renderer->DrawArrays(0, 6);
			renderer->UnbindVertexArray();
		}
	}
}