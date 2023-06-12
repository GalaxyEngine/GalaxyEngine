#include "pch.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "Wrapper/Renderer.h"
#include "Wrapper/Window.h"
#include "Resource/Texture.h"

std::unique_ptr<Wrapper::Renderer> Wrapper::Renderer::m_instance = nullptr;

void Wrapper::Renderer::CreateInstance(RenderAPI renderAPI)
{
	switch (renderAPI)
	{
	case RenderAPI::OPENGL:
	{
		m_instance = std::make_unique<Wrapper::OpenGLRenderer>();
		break;
	}
	default:
		break;
	}
	m_instance->Initalize();
}

// OpenGL Renderer
Wrapper::OpenGLRenderer::OpenGLRenderer(){}

Wrapper::OpenGLRenderer::~OpenGLRenderer(){}


void GALAXY::Wrapper::OpenGLRenderer::Initalize()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		PrintError("Failed to initialize GLAD");
		return;
	}
}

void Wrapper::OpenGLRenderer::CreateTexture(Resource::Texture* texture)
{
	uint32_t filter = TextureFilteringToAPI(texture->m_filtering);
	uint32_t format = TextureFormatToAPI(texture->m_format);

	glGenTextures(1, &texture->m_id);
	glBindTexture(GL_TEXTURE_2D, texture->m_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, filter);

	glTexImage2D(GL_TEXTURE_2D, 0, format, texture->m_size.x, texture->m_size.y, 0, format, GL_UNSIGNED_BYTE, texture->m_bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Wrapper::OpenGLRenderer::DestroyTexture(Resource::Texture* texture)
{
	glDeleteTextures(1, &texture->m_id);
}

uint32_t Wrapper::OpenGLRenderer::TextureFilteringToAPI(Resource::TextureFiltering filtering)
{
	switch (filtering)
	{
	case Resource::TextureFiltering::REPEAT:
		return GL_REPEAT;
	case Resource::TextureFiltering::MIRRORED_REPEAT:
		return GL_MIRRORED_REPEAT;
	case Resource::TextureFiltering::CLAMP_TO_EDGE:
		return GL_CLAMP_TO_EDGE;
	case Resource::TextureFiltering::CLAMP_TO_BORDER:
		return GL_CLAMP_TO_BORDER;
	default:
		PrintError("Texture filtering not recognize");
		return GL_REPEAT;
	}
}

uint32_t Wrapper::OpenGLRenderer::TextureFormatToAPI(Resource::TextureFormat format)
{
	switch (format) {
	case Resource::TextureFormat::RGB:
		return GL_RGB;
	case Resource::TextureFormat::RGBA:
		return GL_RGBA;
	case Resource::TextureFormat::ALPHA:
		return GL_ALPHA;
	case Resource::TextureFormat::LUMINANCE:
		return GL_LUMINANCE;
	case Resource::TextureFormat::LUMINANCE_ALPHA:
		return GL_LUMINANCE_ALPHA;
	default:
		PrintError("Texture format not recognize");
		return GL_RGBA;
	}
}
