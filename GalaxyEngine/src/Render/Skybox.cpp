#include "pch.h"
#include "Render/Skybox.h"

#include "Resource/ResourceManager.h"
#include "Resource/Cubemap.h"

namespace GALAXY
{
    Unique<Render::Skybox> Render::Skybox::m_instance;
    static float skyboxVertices[] = {
        // positions          
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    void Render::Skybox::Initialize()
    {
        m_instance = std::make_unique<Render::Skybox>();
        m_instance->m_skyboxShader = Resource::ResourceManager::GetOrLoad<Resource::Shader>(SKYBOX_SHADER_PATH);

        // skybox mesh setup
        auto renderer = Wrapper::Renderer::GetInstance();
        renderer->CreateVertexArray(m_instance->skyboxVAO);
        renderer->BindVertexArray(m_instance->skyboxVAO);
        renderer->CreateVertexBuffer(m_instance->skyboxVBO, skyboxVertices, sizeof(skyboxVertices));
        renderer->VertexAttribPointer(0, 3, 3 * sizeof(float), (void*)0);
    }

    void Render::Skybox::Render(const Mat4& view, const Mat4& projection, Weak<Resource::Cubemap> cubemap)
    {
        static auto renderer = Wrapper::Renderer::GetInstance();
        auto skyboxShader = m_instance->m_skyboxShader.lock();
        if (!skyboxShader || !skyboxShader->IsLoaded() || !skyboxShader->HasBeenSent())
            return;

        Shared<Resource::Cubemap> sharedCubemap = cubemap.lock();
        if (!sharedCubemap->IsLoaded() || !sharedCubemap->HasBeenSent())
            return;

        Mat4 mat = view;
        mat[3] = Vec3f(0); // Remove translation
        Mat4 vp = projection * mat;

        skyboxShader->Use();
        renderer->EnableDepth(false);

        skyboxShader->SendMat4("VP", vp);
        Wrapper::Renderer::GetInstance()->BindCubemap(sharedCubemap.get());
        skyboxShader->SendInt("skybox", 0);

        renderer->BindVertexArray(m_instance->skyboxVAO);
        renderer->DrawArrays(0, 36);
        renderer->UnbindVertexArray();
        renderer->EnableDepth(true);
    }
}
