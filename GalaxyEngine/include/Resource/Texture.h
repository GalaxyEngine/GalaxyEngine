#pragma once
#include <GalaxyAPI.h>
#include "Resource/IResource.h"
namespace GALAXY {
	namespace Wrapper { 
		class Renderer;

		namespace RendererAPI
		{
			class OpenGLRenderer;
		}
		class Window;
		struct Image;
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

		enum class TextureWrapping
		{
			REPEAT,
			MIRRORED_REPEAT,
			CLAMP_TO_EDGE,
			CLAMP_TO_BORDER,
		};

		inline const char* SerializeTextureWrappingEnum()
		{
			return "Repeat\0Mirrored repeat\0Clamp to edge\0Clamp to border\0";
		}

		enum class TextureFiltering
		{
			NEAREST,
			LINEAR
		};

		inline const char* SerializeTextureFilteringEnum()
		{
			return "Nearest\0Linear\0";
		}

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

			void Save();

			void ShowInInspector() override;

			const char* GetResourceName() const override { return "Texture"; }
			inline Path GetThumbnailPath() const override { return p_fileInfo.GetFullPath(); }

			void Bind(uint32_t index = 0);
			static void UnBind();

			// Get the enum with the class
			static inline ResourceType GetResourceType() { return ResourceType::Texture; }

			static void CreateWithData(const Path& path, const Wrapper::Image& image, TextureFiltering filtering = TextureFiltering::LINEAR, TextureFormat format = TextureFormat::RGBA);
		private:
			void Serialize(CppSer::Serializer& serializer) const override;
			void Deserialize(CppSer::Parser& parser) override;

		private:
			friend Wrapper::Renderer;
			friend Wrapper::RendererAPI::OpenGLRenderer;
			friend Wrapper::Window;

			uint32_t m_id = -1;

			TextureFiltering m_filtering = TextureFiltering::LINEAR;
			TextureFormat m_format = TextureFormat::RGBA;
			TextureWrapping m_wrapping = TextureWrapping::REPEAT;

			unsigned char* m_bytes = nullptr;
			Vec2i m_size = {};

			bool m_generateMipmaps = false;
		};
	}
}