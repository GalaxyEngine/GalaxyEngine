#pragma once
#include <array>

#include "GalaxyAPI.h"
#include "IResource.h"

namespace GALAXY 
{
    namespace Wrapper
    {
        namespace RendererAPI
        {
            class OpenGLRenderer;
        }
    }
    namespace Resource
    {
        class Cubemap;
        class Cubemap;
        class CubemapTexture
        {
        public:
            explicit CubemapTexture(Cubemap* owner);
            CubemapTexture& operator=(const CubemapTexture& other) {}
            CubemapTexture(const CubemapTexture&) = default;
            CubemapTexture(CubemapTexture&&) noexcept = default;
            ~CubemapTexture();
            
            
            virtual void ShowOnInspector();
            virtual void Load(CppSer::Parser& parser);
            virtual void Send(Resource::Cubemap* cubemap);
            virtual void Save(CppSer::Serializer& serializer) const;

            void UpdateCubemap();

        protected:
            Cubemap* p_owner = nullptr;
        private:
            Weak<Texture> m_texture;

            // Temporary variable, delete when the cube map is sent to the render api
            Wrapper::Image* m_image = nullptr;
        };

        class SixSidedTexture : public CubemapTexture
        {
        public:
            explicit SixSidedTexture(Cubemap* owner);
            SixSidedTexture& operator=(const SixSidedTexture& other) = default;
            SixSidedTexture(const SixSidedTexture&) = default;
            SixSidedTexture(SixSidedTexture&&) noexcept = default;
            ~SixSidedTexture();

            void ShowOnInspector() override;

            void Load(CppSer::Parser& parser) override;
            void Send(Resource::Cubemap* cubemap) override;
            void Save(CppSer::Serializer& serializer) const;

            void UpdateFace(uint32_t index, const Path& path);
            void UpdateFace(uint32_t index, const Weak<Texture>& texture);
            
        private:
            friend Wrapper::RendererAPI::OpenGLRenderer;
            
            // direction : +X, -X, +Y, -Y, +Z, -Z
            std::array<Weak<Texture>, 6> m_textures;
            
        };
        
        enum class CubemapType
        {
            Default,
            SixSided,
            Panoramic
        };
        
        inline const char* SerializeCubemapTypeEnum()
        {
            return "Default\0SixSided\0";
        }
        
        class Cubemap : public IResource
        {
        public:
            explicit Cubemap(const Path& fullPath);
            Cubemap& operator=(const Cubemap& other);
            Cubemap(const Cubemap&) = default;
            Cubemap(Cubemap&&) noexcept = default;
            ~Cubemap() override;

            const char* GetResourceName() const override { return "Cubemap"; }
            Path GetThumbnailPath() const override;
            static inline ResourceType GetResourceType() { return ResourceType::Cubemap; }

            void Load() override;
            void Send() override;
            void Save() const;

            inline uint32_t GetID() const { return m_id; }

            static std::string GetDirectionFromIndex(uint32_t index);

            static Weak<Cubemap> Create(const Path& path);

            void SetType(CubemapType type);

            void ShowInInspector() override;
        private:
            friend Wrapper::RendererAPI::OpenGLRenderer;
            
			uint32_t m_id = -1;
            
            // direction : +X, -X, +Y, -Y, +Z, -Z
            // std::array<Weak<Texture>, 6> m_textures;

            CubemapType m_type = CubemapType::Default;
            Unique<CubemapTexture> m_texture;
        };
    }
}
