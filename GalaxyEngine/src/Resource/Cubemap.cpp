#include "pch.h"
#include "Resource/Texture.h"
#include "Resource/Cubemap.h"

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

    Path Resource::Cubemap::GetThumbnailPath() const
    {
        if (m_textures[2].lock())
        {
            return m_textures[2].lock()->GetThumbnailPath();
        }
        return Path();
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


        for (uint32_t i = 0; i < 6; i++)
        {
            auto textureUUID = parser[GetDirectionFromIndex(i)].As<uint64_t>();
            m_textures[i] = Resource::ResourceManager::GetResource<Texture>(textureUUID);
        }

        if (!std::filesystem::exists(GetDataFilePath()))
            CreateDataFile();

        SendRequest();
        FinishLoading();
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
                Wrapper::Image image = Wrapper::ImageLoader::Load(texture->GetFileInfo().GetFullPath().string().c_str(),
                                                                  4);
                Wrapper::Renderer::GetInstance()->SetCubemapFace(i, image);
                Wrapper::ImageLoader::ImageFree(image);
            }
            else
            {
                Wrapper::Renderer::GetInstance()->SetCubemapFace(i, Wrapper::Image());
            }
        }

        Wrapper::Renderer::GetInstance()->SetCubemapParameters();
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
            Wrapper::Image image = Wrapper::ImageLoader::Load(
                texture.lock()->GetFileInfo().GetFullPath().string().c_str(), 4);
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
