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
		class Mesh;
		class PostProcessShader;
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

			// Update the size of the Framebuffer each frame
			void Update(const Vec2i& windowSize);
			void Render();

			void SetPostProcessShader(Weak<Resource::PostProcessShader> postProcessShader);
			Weak<Resource::PostProcessShader> GetPostProcessShader();
		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;

			std::shared_ptr<Framebuffer> m_postProcess;

			Weak<Resource::PostProcessShader> m_shader;
			Weak<Resource::Mesh> m_plane;

			uint32_t m_renderBuffer = -1;
			uint32_t m_frameBuffer = -1;

			uint32_t m_index = -1;

			Wrapper::Renderer* m_renderer = nullptr;

			Vec2i m_size;
			std::shared_ptr<Resource::Texture> m_renderTexture;
		};
	}
}
#include "Render/FrameBuffer.inl" 
