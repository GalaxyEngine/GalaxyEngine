#include "pch.h"
#include "Render/ShadowMap.h"

// TODO : Remove
#include <glad/glad.h>

#include "Core/Application.h"
#include <iostream>  // For error logging

#include "Render/Framebuffer.h"
#include "Resource/ResourceManager.h"

namespace GALAXY 
{
    void Render::ShadowMap::Initialize()
    {

        m_index = Framebuffer::GetFreeIndex();

        // Create depth texture
        m_renderTexture = std::make_shared<Resource::Texture>("ShadowMap" + std::to_string(m_index) + ".png");
        m_renderTexture->m_size = m_resolution;
        m_renderTexture->p_shouldBeLoaded = true;
        m_renderTexture->p_loaded = true;
        m_renderTexture->p_hasBeenSent = true;
        Resource::ResourceManager::AddResource(m_renderTexture);
        
        // Configure depth map FBO
        glGenFramebuffers(1, &m_FBO);
        
        glGenTextures(1, &m_renderTexture->m_id);
        glBindTexture(GL_TEXTURE_2D, m_renderTexture->m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_renderTexture->m_size.x, m_renderTexture->m_size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_renderTexture->m_id, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_initialized = true;
    }

    void Render::ShadowMap::Begin()
    {
        if (!m_initialized)
            Initialize();
        
        // Bind the shadow map framebuffer and set the viewport to the shadow map resolution
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        
        if (m_shouldUpdateResolution) {
            glBindTexture(GL_TEXTURE_2D, m_renderTexture->m_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_resolution.x, m_resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        glViewport(0, 0, m_resolution.x, m_resolution.y);
        // Clear only the depth buffer (color buffer is not used)
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void Render::ShadowMap::End()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        auto renderer = Wrapper::Renderer::GetInstance();
        renderer->SetViewport(Core::Application::GetInstance().GetWindow()->GetSize());
    }


#ifdef WITH_EDITOR
    void Render::ShadowMap::ShowInInspector()
    {
        if (ImGui::TreeNodeEx("Shadow Map", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Enable", &m_enable);
            int res = m_resolution.x;
            if (ImGui::InputInt("Resolution", &res))
            {
                SetResolution(Vec2f(static_cast<float>(res)));
            }
		    ImGui::DragFloatRange2("Near/Far", &m_near, &m_far, 0.1f);
            if (ImGui::TreeNodeEx("Texture", ImGuiTreeNodeFlags_DefaultOpen))
            {
                float imageSize = ImGui::GetContentRegionAvail().x * 0.5f;
                Wrapper::GUI::TextureImage(m_renderTexture.get(), Vec2f(imageSize), Vec2f(0, 1), Vec2f(1, 0));
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }
#endif
}
