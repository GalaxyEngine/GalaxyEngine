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
			void RenderPostProcess(const Vec2i& size, const Vec4f& clearColor);

			void Begin(const Vec2i& size);
			void End(const Vec2i& size, const Vec4f& clearColor);

			inline Weak<Resource::PostProcessShader> GetPostProcessShader() const;

			void SetPostProcessShader(const Weak<Resource::PostProcessShader>& postProcessShader);
		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;

			std::shared_ptr<Framebuffer> m_postProcess;

			Weak<Resource::PostProcessShader> m_shader;
			Weak<Resource::Mesh> m_plane;
			Shared<Resource::Material> m_renderMaterial;

			uint32_t m_renderBuffer = -1;
			uint32_t m_frameBuffer = -1;
			uint32_t m_index = -1;

			Vec2i m_size;
			std::shared_ptr<Resource::Texture> m_renderTexture;
		};
	}
}
#include "Framebuffer.inl" 
