#include "pch.h"
#include "Render/Grid.h"
#include "Render/Camera.h"

#include "Resource/ResourceManager.h"
#include "Resource/ResourceManager.h"

#include "Core/SceneHolder.h"

#include <glad/glad.h>

#define GRID_PATH ENGINE_RESOURCE_FOLDER_NAME"\\shaders\\GridShader\\grid.shader"
namespace GALAXY
{

	void Render::Grid::Initialize()
	{
		m_shader = Resource::ResourceManager::GetInstance()->GetOrLoad<Resource::Shader>(GRID_PATH);

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Render::Grid::Draw()
	{
		if (auto gridShader = m_shader.lock(); gridShader && gridShader->HasBeenSent()) {

			const auto renderer = Wrapper::Renderer::GetInstance();
			const auto currentCamera = Render::Camera::GetCurrentCamera().lock();

			renderer->BindVertexArray(vao);

			gridShader->Use();

			gridShader->SendMat4("view", currentCamera->GetViewMatrix());
			gridShader->SendMat4("proj", currentCamera->GetProjectionMatrix());
			gridShader->SendFloat("near", currentCamera->GetNear());
			gridShader->SendFloat("far", currentCamera->GetFar());

			renderer->DrawArrays(0, 6);
			renderer->UnbindVertexArray();
		}
	}
}