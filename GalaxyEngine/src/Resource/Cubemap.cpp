#include "pch.h"
#include "Resource/Texture.h"
#include "Resource/Cubemap.h"

#include <glad/glad.h>

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

        uint32_t i = 0;
        for (const auto& pair : parser.GetValueMap()[parser.GetCurrentDepth()])
        {
            auto textureUUID = pair.second.As<uint64_t>();
            m_textures[i++] = Resource::ResourceManager::GetResource<Texture>(textureUUID);
        }
        
        if (!std::filesystem::exists(GetDataFilePath()))
            CreateDataFile();

        SendRequest();
        FinishLoading();
        //TODO init with opengl cubemap
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
                Wrapper::Image image = Wrapper::ImageLoader::Load(texture->GetFileInfo().GetFullPath().string().c_str(), 0);
                // Wrapper::Renderer::GetInstance()->SetCubemapFace(i, image);
                glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, image.size.x, image.size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);
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
