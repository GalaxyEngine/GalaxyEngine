#include "pch.h"
#include "Resource/Texture.h"
#include "Wrapper/ImageLoader.h"
#include "Core/Application.h"

Resource::Texture::~Texture()
{
	Wrapper::Renderer::GetInstance()->DestroyTexture(this);
	if (p_shouldBeLoaded && !p_hasBeenSent)
		Wrapper::ImageLoader::ImageFree(this->m_bytes);
}

void Resource::Texture::Load()
{
	if (p_shouldBeLoaded)
		return;
	p_shouldBeLoaded = true;
	int numColCh;
	if ((m_bytes = Wrapper::ImageLoader::Load(p_fileInfo.GetFullPath().string().c_str(), &m_size.x, &m_size.y, &numColCh, 4)))
		p_loaded = true;
	else 
	{
		PrintError("Failed to load Image %s", p_fileInfo.GetFullPath().string().c_str());
		return;
	}
	SendRequest();
}

void Resource::Texture::Send()
{
	if (p_hasBeenSent)
		return;
	Wrapper::Renderer::GetInstance()->CreateTexture(this);
	Wrapper::ImageLoader::ImageFree(this->m_bytes);
	p_hasBeenSent = true;
}

void Resource::Texture::Bind(const uint32_t index /* = 0 */)
{
	Wrapper::Renderer::GetInstance()->BindTexture(this, index);
}

void Resource::Texture::UnBind()
{
	Wrapper::Renderer::GetInstance()->UnbindTexture();
}
