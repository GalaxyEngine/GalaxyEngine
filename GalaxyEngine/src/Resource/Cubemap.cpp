#include "pch.h"
#include "Resource/Texture.h"
#include "Resource/Cubemap.h"

#include <glad/glad.h>

#include "Core/SceneHolder.h"
#include "Render/Camera.h"
#include "Resource/ResourceManager.h"
#include "Wrapper/ImageLoader.h"


namespace GALAXY 
{
    Resource::Cubemap::Cubemap(const Path& fullPath) : IResource(fullPath)
    {
    }

    Resource::Cubemap::~Cubemap()
    {
        Wrapper::Renderer::GetInstance()->DestroyCubemap(this);
    }

    static float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    void Resource::Cubemap::Load()
    {
        if (p_loaded)
            return;
        p_loaded = true;
        
        CppSer::Parser parser(p_fileInfo.GetFullPath());
        if (!parser.IsFileOpen())
        {
            PrintError("Cubemap file not found : %s", p_fileInfo.GetFullPath().string().c_str());
            return;
        }

        
        for (uint32_t i = 0; i < 6; i++)
        {
            auto textureUUID = parser[GetDirectionFromIndex(i)].As<uint64_t>();
            m_textures[i] = Resource::ResourceManager::GetResource<Texture>(textureUUID);
        }
        
        if (!std::filesystem::exists(GetDataFilePath()))
            CreateDataFile();

        SendRequest();
        FinishLoading();

        //TODO init with opengl cubemap
        m_skyboxShader = Resource::ResourceManager::GetResource<Shader>(SKYBOX_PATH);
    }

    void Resource::Cubemap::Send()
    {
        if (p_hasBeenSent)
            return;
        p_hasBeenSent = true;
        Wrapper::Renderer::GetInstance()->CreateCubemap(this);
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            if (!m_textures[i].lock())
                return;
            if (auto texture = m_textures[i].lock())
            {
                Wrapper::Image image = Wrapper::ImageLoader::Load(texture->GetFileInfo().GetFullPath().string().c_str(), 4);
                Wrapper::Renderer::GetInstance()->SetCubemapFace(i, image);
                Wrapper::ImageLoader::ImageFree(image);
            }
            else
            {
                Wrapper::Renderer::GetInstance()->SetCubemapFace(i, Wrapper::Image());
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Todo move to skybox class
        // skybox VAO
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    void Resource::Cubemap::Save() const
    {
		CppSer::Serializer serializer(p_fileInfo.GetFullPath());

        serializer << CppSer::Pair::BeginMap << "Cubemap";

        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            SerializeResource(serializer, GetDirectionFromIndex(i).c_str(), m_textures[i]);
        }
        
        serializer << CppSer::Pair::EndMap << "Cubemap";
    }

    void Resource::Cubemap::UpdateFace(uint32_t index, const Path& path)
    {
       UpdateFace(index, Resource::ResourceManager::GetOrLoad<Texture>(path));
    }

    void Resource::Cubemap::UpdateFace(uint32_t index, const Weak<Texture>& texture)
    {
        m_textures[index] = texture;
        if (texture.lock())
        {
            Wrapper::Image image = Wrapper::ImageLoader::Load(texture.lock()->GetFileInfo().GetFullPath().string().c_str(), 4);
            Wrapper::Renderer::GetInstance()->SetCubemapFace(index, image);
            Wrapper::ImageLoader::ImageFree(image);
        }
        else
        {
            Wrapper::Renderer::GetInstance()->SetCubemapFace(index, Wrapper::Image());
        }
        Save();
    }

    void Resource::Cubemap::RenderCubemap()
    {
        if (!m_skyboxShader.lock() || !m_skyboxShader.lock()->IsLoaded() || !m_skyboxShader.lock()->HasBeenSent())
            return;

        if (!p_loaded || !p_hasBeenSent)
            return;

        auto skyboxShader = m_skyboxShader.lock();
        skyboxShader->Use();
	    glDisable(GL_DEPTH_TEST);
        auto camera = Core::SceneHolder::GetCurrentScene()->GetCurrentCamera();

        auto mat = camera->GetViewMatrix();
        mat[3] = Vec3f(0); // Remove translation
        skyboxShader->SendMat4("VP",  camera->GetProjectionMatrix() * mat);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
        skyboxShader->SendInt("skybox", 0);
        
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
	    glEnable(GL_DEPTH_TEST);
    }

    Weak<Resource::Cubemap> Resource::Cubemap::Create(const Path& path)
    {
        auto cubemap = Resource::ResourceManager::AddResource<Cubemap>(path);
        cubemap.lock()->Save();
        return cubemap;
    }

    void Resource::Cubemap::ShowInInspector()
    {
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            Resource::ResourceManager::ResourceField(m_textures[i], GetDirectionFromIndex(i));
        }
        if (ImGui::Button("Save"))
        {
            Save();
        }
    }

    std::string Resource::Cubemap::GetDirectionFromIndex(uint32_t index)
    {
        switch (index)
        {
        case 0:
            return "Right";
        case 1:
            return "Left";
        case 2:
            return "Top";
        case 3:
            return "Bottom";
        case 4:
            return "Front";
        case 5:
            return "Back";
        default:
            return "Unknown";
        }
    }
}
