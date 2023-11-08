#include "pch.h"
#include "Render/Framebuffer.h"
#include "Render/Camera.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

#define PLANE_PATH ENGINE_RESOURCE_FOLDER_NAME"/models/Plane.obj:Plane"
namespace GALAXY {

	static std::unordered_map<size_t, bool> s_indexArray = {};
	Render::Framebuffer::Framebuffer(const Vec2i& size) : m_size(size)
	{
		m_renderer = Wrapper::Renderer::GetInstance();
		// Check for free index in list of indices
		int freeIndex = 0;
		while (s_indexArray.count(freeIndex) > 0) {
			freeIndex++;
		}
		m_index = freeIndex;
		s_indexArray[m_index] = true;

		m_renderTexture = std::make_shared<Resource::Texture>("Framebuffer_" + std::to_string(m_index) + ".png");
		Resource::ResourceManager::GetInstance()->AddResource(m_renderTexture);

		Wrapper::Renderer::GetInstance()->CreateRenderBuffer(this);
		m_plane = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(PLANE_PATH);
	}

	Render::Framebuffer::~Framebuffer()
	{
		Wrapper::Renderer::GetInstance()->DeleteRenderBuffer(this);
		s_indexArray.erase(m_index);
		Resource::ResourceManager::GetInstance()->RemoveResource(m_renderTexture->GetFileInfo().GetRelativePath());
		m_renderTexture.reset();
	}

	std::weak_ptr<GALAXY::Resource::Texture> Render::Framebuffer::GetRenderTexture() const
	{
		if (!m_postProcess)
			return m_renderTexture;
		return m_postProcess->GetRenderTexture();
	}

	void Render::Framebuffer::Update(const Vec2i& windowSize)
{
		if (m_size != windowSize) {
			m_size = windowSize;
			Wrapper::Renderer::GetInstance()->BindRenderBuffer(this);
			m_renderTexture->Bind();
			Wrapper::Renderer::GetInstance()->ResizeRenderBuffer(this, m_size);
			m_renderTexture->UnBind();
			Wrapper::Renderer::GetInstance()->UnbindRenderBuffer(this);
		}
	}

	void Render::Framebuffer::RenderPostProcess()
	{
		if (!m_postProcess)
			return;
		auto postProcessFramebuffer = m_postProcess.get();

		m_renderer->BindRenderBuffer(postProcessFramebuffer);

		m_renderer->ClearColorAndBuffer(m_clearColor);

		m_plane.lock()->Render(Mat4(), { m_renderMaterial });

		m_renderer->UnbindRenderBuffer(postProcessFramebuffer);
	}

	void Render::Framebuffer::Begin()
	{
		Update(Core::Application::GetInstance().GetWindow()->GetSize());
		Wrapper::Renderer::GetInstance()->BindRenderBuffer(this);
	}

	void Render::Framebuffer::End()
	{
		Wrapper::Renderer::GetInstance()->UnbindRenderBuffer(this);
		RenderPostProcess();
	}

	void Render::Framebuffer::SetPostProcessShader(Weak<Resource::PostProcessShader> postProcessShader)
	{
		if (!postProcessShader.lock())
		{
			m_postProcess.reset();
			return;
		}
		if (!m_postProcess)
			m_postProcess = std::make_shared<Framebuffer>(Core::Application::GetInstance().GetWindow()->GetSize());
		if (!m_renderMaterial) {
			m_renderMaterial = std::make_shared<Resource::Material>("RenderMaterial");
			m_renderMaterial->SetAlbedo(m_renderTexture);
			m_renderMaterial->p_shouldBeLoaded = true;
			m_renderMaterial->p_loaded = true;
			m_renderMaterial->p_hasBeenSent = true;
		}
		m_renderMaterial->SetShader(postProcessShader);
		m_postProcess->m_shader = postProcessShader;
	}

}