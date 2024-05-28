#pragma once
#include <array>

#include "GalaxyAPI.h"
#include "IResource.h"

namespace GALAXY 
{

    namespace Resource
    {
        class Cubemap : public IResource
        {
        public:
            explicit Cubemap(const Path& fullPath);
            Cubemap& operator=(const Cubemap& other);
            Cubemap(const Cubemap&) = default;
            Cubemap(Cubemap&&) noexcept = default;
            ~Cubemap() override;

            const char* GetResourceName() const override { return "Cubemap"; }
            static inline ResourceType GetResourceType() { return ResourceType::Cubemap; }

            void Load() override;
            void Send() override;
            void Save() const;

            void UpdateFace(uint32_t index, const Path& path);
            void UpdateFace(uint32_t index, const Weak<Texture>& texture);

            // Todo move to skybox class
            void RenderCubemap();

            static Weak<Cubemap> Create(const Path& path);

            void ShowInInspector();
        private:
            static std::string GetDirectionFromIndex(uint32_t index);
        private:
            friend Wrapper::RendererAPI::OpenGLRenderer;
            
			uint32_t m_id = -1;
            
            // direction : +X, -X, +Y, -Y, +Z, -Z
            std::array<Weak<Texture>, 6> m_textures;

            // Todo move to skybox class
            uint32_t skyboxVAO, skyboxVBO;
            Weak<Shader> m_skyboxShader;
        };
    }
}
