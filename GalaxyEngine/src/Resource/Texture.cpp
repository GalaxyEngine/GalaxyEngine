#include "pch.h"
#include "Resource/Texture.h"
#include "Wrapper/ImageLoader.h"

void Resource::Texture::Load()
{
	p_shouldBeLoaded = true;
	int numColCh;
	if (m_bytes = Wrapper::ImageLoader::Load(p_fullPath.c_str(), &m_size.x, &m_size.y, &numColCh, 4))
		p_loaded = true;
	else
		PrintError("Failed to load Image %s", p_fullPath.c_str());
}

void Resource::Texture::Send()
{
	//TODO	
}
