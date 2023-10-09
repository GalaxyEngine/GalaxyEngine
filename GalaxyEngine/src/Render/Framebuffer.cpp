#include "pch.h"
#include "Render/Framebuffer.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

#include "Core/Application.h"

namespace GALAXY {

	static std::unordered_map<size_t, bool> s_indexArray = {};
	Render::Framebuffer::Framebuffer(const Vec2i& size) : m_size(size)
	{
		// Check for free index in list of indices
		int freeIndex = 0;
		while (s_indexArray.count(freeIndex) > 0) {
			freeIndex++;
		}
		m_index = freeIndex;
		s_indexArray[m_index] = true;

		m_renderTexture = std::make_shared<Resource::Texture>("Framebuffer");
		Resource::ResourceManager::GetInstance()->AddResource(m_renderTexture.get());

		Wrapper::Renderer::GetInstance()->CreateRenderBuffer(this);
	}

	Render::Framebuffer::~Framebuffer()
	{
		Wrapper::Renderer::GetInstance()->DeleteRenderBuffer(this);
		s_indexArray.erase(m_index);
		Resource::ResourceManager::GetInstance()->RemoveResource(m_renderTexture.get());
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
		if (m_postProcess)
		{

		}
	}

	void Render::Framebuffer::SetPostProcessShader(Weak<Resource::PostProcessShader> postProcessShader)
	{
		if (!m_postProcess)
			m_postProcess = std::make_shared<Framebuffer>(Core::Application::GetInstance().GetWindow()->GetSize());
		m_postProcess->m_shader = postProcessShader;
	}

	Weak<Resource::PostProcessShader> Render::Framebuffer::GetPostProcessShader()
	{
		if (!m_postProcess)
			return Weak<Resource::PostProcessShader>();
		return m_postProcess->m_shader;
	}

}