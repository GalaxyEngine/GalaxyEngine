#pragma once
#include "GalaxyAPI.h"

namespace GALAXY
{
	namespace Wrapper
	{
		class Renderer;
		class OpenGLRenderer;
	}
	namespace Resource
	{
		class Texture;
	}
	namespace Render
	{
		class Framebuffer
		{
		public:
			Framebuffer(const Vec2i& size);
			Framebuffer& operator=(const Framebuffer& other) = default;
			Framebuffer(const Framebuffer&) = default;
			Framebuffer(Framebuffer&&) noexcept = default;
			virtual ~Framebuffer();

			std::weak_ptr<GALAXY::Resource::Texture> GetRenderTexture() const { return m_renderTexture; }
		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;

			uint32_t m_renderBuffer = -1;
			uint32_t m_frameBuffer = -1;

			uint32_t m_index = -1;

			Vec2i m_size;
			std::shared_ptr<Resource::Texture> m_renderTexture;
		};
	}
}
#include "Render/FrameBuffer.inl" 
