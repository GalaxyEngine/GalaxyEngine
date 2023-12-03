#pragma once
#include <GalaxyAPI.h>
#include "Resource/IResource.h"
namespace GALAXY {
	namespace Wrapper { 
		class Renderer;
		class OpenGLRenderer;
	}
	namespace Resource {

		enum class TextureFormat
		{
			RGB,
			RGBA,
			ALPHA,
			LUMINANCE,
			LUMINANCE_ALPHA
		};

		enum class TextureFiltering
		{
			REPEAT,
			MIRRORED_REPEAT,
			CLAMP_TO_EDGE,
			CLAMP_TO_BORDER,
		};

		class Texture : public IResource
		{
		public:
			explicit Texture(const Path& fullPath) : IResource(fullPath) {}
			Texture& operator=(const Texture& other) = default;
			Texture(const Texture&) = default;
			Texture(Texture&&) noexcept = default;
			~Texture() override;

			inline uint32_t GetID() const { return m_id; }

			void Load() override;
			void Send() override;

			void Bind(uint32_t index = 0);
			static void UnBind();

			// Get the enum with the class
			static inline ResourceType GetResourceType() { return ResourceType::Texture; }

		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;

			uint32_t m_id = -1;
			unsigned char* m_bytes = nullptr;
			Vec2i m_size = {};
			TextureFormat m_format = TextureFormat::RGBA;
			TextureFiltering m_filtering = TextureFiltering::REPEAT;
			bool m_generateMipmaps = false;
		};
	}
}