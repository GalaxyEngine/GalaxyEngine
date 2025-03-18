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

            void SetResolution(const Vec2i& resolution) { m_resolution = resolution; m_shouldUpdateResolution = true;}
            
            Shared<Resource::Texture> GetRenderTexture() const {return m_renderTexture;}
            Vec2i GetResolution() const { return m_resolution; }
            float GetNear() const { return m_near; }
            float GetFar() const { return m_far; }

#ifdef WITH_EDITOR
            EDITOR_ONLY void ShowInInspector();
#endif
            bool IsEnabled() const { return m_enable; }
            
        private:
            unsigned int m_FBO;
            unsigned int m_depthMap;
            size_t m_index;
            Shared<Resource::Texture> m_renderTexture = nullptr;
            bool m_initialized = false;
            bool m_shouldUpdateResolution = false;
            
            bool m_enable = true;
            Vec2i m_resolution = { 1024, 1024 };
            float m_near = 1.f, m_far = 7.5f;

        };
    }
}
