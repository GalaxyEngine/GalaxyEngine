#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
    namespace Render
    {
        class GALAXY_API ShadowMap
        {
        public:
            ShadowMap() = default;
            ShadowMap& operator=(const ShadowMap& other) = default;
            ShadowMap(const ShadowMap&) = default;
            ShadowMap(ShadowMap&&) noexcept = default;
            virtual ~ShadowMap() = default;

            void Initialize();

            void Begin();
            void End();
            
            Shared<Resource::Texture> GetRenderTexture() const {return m_renderTexture;}

        private:
            unsigned int m_FBO;
            unsigned int m_depthMap;
            Vec2i m_resolution = { 1024, 1024 };
            size_t m_index;
            Shared<Resource::Texture> m_renderTexture = nullptr;
            bool m_initialized = false;
        };
    }
}
