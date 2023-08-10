#include "pch.h"
#include "Render/Framebuffer.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

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

		m_renderTexture = std::make_shared<Resource::Texture>("Framebuffer ");
		Resource::ResourceManager::GetInstance()->AddResource(m_renderTexture.get());

		Wrapper::Renderer::GetInstance()->CreateRenderBuffer(this);
	}

	Render::Framebuffer::~Framebuffer()
	{
		Wrapper::Renderer::GetInstance()->DeleteRenderBuffer(this);
		s_indexArray.erase(m_index);
		Resource::ResourceManager::GetInstance()->RemoveResource(m_renderTexture.get());
	}

}