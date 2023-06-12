#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Resource { 
		enum class TextureFormat;
		enum class TextureFiltering;
		class Texture;
	}
	namespace Wrapper 
	{
		enum class RenderAPI
		{
			OPENGL,
			VULKAN,
		};


		class Renderer
		{
		public:
			Renderer() {}
			Renderer& operator=(const Renderer& other) = default;
			Renderer(const Renderer&) = default;
			Renderer(Renderer&&) noexcept = default;
			virtual ~Renderer() {}

			static void CreateInstance(RenderAPI renderAPI);
			static Renderer* GetInstance() { return m_instance.get(); }

			virtual void Initalize() {}

			// Texture
			virtual void CreateTexture(Resource::Texture* texture) {}
			virtual void DestroyTexture(Resource::Texture* texture) {}

			virtual uint32_t TextureFormatToAPI(Resource::TextureFormat format) { return -1; }
			virtual uint32_t TextureFilteringToAPI(Resource::TextureFiltering filtering) { return -1; }

		private:
			static std::unique_ptr<Renderer> m_instance;
		};

		class OpenGLRenderer : public Renderer
		{
		public:
			OpenGLRenderer();
			OpenGLRenderer& operator=(const OpenGLRenderer& other) = default;
			OpenGLRenderer(const OpenGLRenderer&) = default;
			OpenGLRenderer(OpenGLRenderer&&) noexcept = default;
			virtual ~OpenGLRenderer();

			void Initalize() override;

			// Texture
			void CreateTexture(Resource::Texture* texture);
			void DestroyTexture(Resource::Texture* texture);

			uint32_t TextureFormatToAPI(Resource::TextureFormat format) override;
			uint32_t TextureFilteringToAPI(Resource::TextureFiltering filtering) override;
		private:

		};
	}
}