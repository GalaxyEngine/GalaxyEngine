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
		class Material;
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

			std::weak_ptr<Resource::Texture> GetRenderTexture() const;

			// Update the size of the Framebuffer each frame
			void Update(const Vec2i& windowSize);
			void RenderPostProcess();

			void Begin();
			void End();

			inline Weak<Resource::PostProcessShader> GetPostProcessShader();

			void SetPostProcessShader(Weak<Resource::PostProcessShader> postProcessShader);
			inline void SetClearColor(GALAXY::Math::Vec4f val) { m_clearColor = val; }
		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;

			Vec4f m_clearColor = Vec4f(0.0f, 0.0f, 0.0f, 0.0f);

			std::shared_ptr<Framebuffer> m_postProcess;

			Weak<Resource::PostProcessShader> m_shader;
			Weak<Resource::Mesh> m_plane;
			Shared<Resource::Material> m_renderMaterial;

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
