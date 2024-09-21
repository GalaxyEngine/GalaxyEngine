#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
    namespace Render
    {
        class Skybox
        {
        public:
            Skybox() = default;
            ~Skybox() = default;

            static void Initialize();

            static void Render(const Mat4& view, const Mat4& projection, Weak<Resource::Cubemap> cubemap);

            static Weak<Resource::Shader> GetShader() { return m_instance->m_skyboxShader; }

        private:
            static Unique<Skybox> m_instance;

            Weak<Resource::Shader> m_skyboxShader;
            
            uint32_t skyboxVAO, skyboxVBO;
        };
    }
}
