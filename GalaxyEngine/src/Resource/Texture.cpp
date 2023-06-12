#include "pch.h"
#include "Resource/Texture.h"
#include "Wrapper/ImageLoader.h"

Resource::Texture::~Texture()
{
	Wrapper::Renderer::GetInstance()->DestroyTexture(this);
}

void Resource::Texture::Load()
{
	p_shouldBeLoaded = true;
	int numColCh;
	if (m_bytes = Wrapper::ImageLoader::Load(p_fullPath.c_str(), &m_size.x, &m_size.y, &numColCh, 4))
		p_loaded = true;
	else
		PrintError("Failed to load Image %s", p_fullPath.c_str());
	Send();
}

void Resource::Texture::Send()
{
	Wrapper::Renderer::GetInstance()->CreateTexture(this);
	Wrapper::ImageLoader::ImageFree(this->m_bytes);
	p_hasBeenSent = true;
}
