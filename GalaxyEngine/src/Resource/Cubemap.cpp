#include "pch.h"
#include "Resource/Texture.h"
#include "Resource/Cubemap.h"

#include "Core/SceneHolder.h"
#include "Resource/ResourceManager.h"
#include "Wrapper/ImageLoader.h"


namespace GALAXY
{
    void Resource::CubemapTexture::ShowOnInspector()
    {
        Resource::ResourceManager::ResourceField(m_texture, "Texture");
    }

    void Resource::CubemapTexture::Load(CppSer::Parser& parser)
    {
        auto textureUUID = parser["Texture"].As<uint64_t>();
        m_texture = ResourceManager::GetResource<Texture>(textureUUID);
        //TODO : Load
    }

    void Resource::CubemapTexture::Send(Resource::Cubemap* cubemap)
    {
        //TODO : Send
    }

    void Resource::CubemapTexture::Save(CppSer::Serializer& serializer) const
    {
        IResource::SerializeResource(serializer, "Texture", m_texture);
        //TODO : Save
    }
#pragma region SixSidedTexture
    void Resource::SixSidedTexture::ShowOnInspector()
    {
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            Resource::ResourceManager::ResourceField(m_textures[i], Cubemap::GetDirectionFromIndex(i));
        }
    }

    void Resource::SixSidedTexture::Load(CppSer::Parser& parser)
    {
        for (uint32_t i = 0; i < 6; i++)
        {
            auto textureUUID = parser[Cubemap::GetDirectionFromIndex(i)].As<uint64_t>();
            m_textures[i] = ResourceManager::GetResource<Texture>(textureUUID);
        }
    }

    void Resource::SixSidedTexture::Send(Resource::Cubemap* cubemap)
    {
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            if (!m_textures[i].lock())
                return;
            if (auto texture = m_textures[i].lock())
            {
                Wrapper::Image image = Wrapper::ImageLoader::Load(texture->GetFileInfo().GetFullPath().string().c_str(),
                                                                  4);
                Wrapper::Renderer::GetInstance()->SetCubemapFace(static_cast<int>(i), image);
                Wrapper::ImageLoader::ImageFree(image);
            }
            else
            {
                Wrapper::Renderer::GetInstance()->SetCubemapFace(static_cast<int>(i), Wrapper::Image());
            }
        }
    }

    void Resource::SixSidedTexture::Save(CppSer::Serializer& serializer) const
    {
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            IResource::SerializeResource(serializer, Cubemap::GetDirectionFromIndex(i).c_str(), m_textures[i]);
        }
    }

    void Resource::SixSidedTexture::UpdateFace(uint32_t index, const Path& path)
    {
        UpdateFace(index, Resource::ResourceManager::GetOrLoad<Texture>(path));
    }

    void Resource::SixSidedTexture::UpdateFace(uint32_t index, const Weak<Texture>& texture)
    {
        m_textures[index] = texture;
        if (texture.lock())
        {
            Wrapper::Image image = Wrapper::ImageLoader::Load(
                texture.lock()->GetFileInfo().GetFullPath().string().c_str(), 4);
            Wrapper::Renderer::GetInstance()->SetCubemapFace(static_cast<int>(index), image);
            Wrapper::ImageLoader::ImageFree(image);
        }
        else
        {
            Wrapper::Renderer::GetInstance()->SetCubemapFace(static_cast<int>(index), Wrapper::Image());
        }
        //Save();
    }
#pragma endregion 

    Resource::Cubemap::Cubemap(const Path& fullPath) : IResource(fullPath)
    {
        SetType(CubemapType::SixSided);
    }

    Resource::Cubemap::~Cubemap()
    {
        Wrapper::Renderer::GetInstance()->DestroyCubemap(this);
    }

    Path Resource::Cubemap::GetThumbnailPath() const
    {
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
            p_loaded = false;
            return;
        }
        if (parser.GetVersion() != "1.0")
        {
            PrintError("Cubemap version %s not supported : %s", parser.GetVersion().c_str(),  p_fileInfo.GetFullPath().string().c_str());
            p_loaded = false;
            return;
        }

        m_type = static_cast<CubemapType>(parser["Type"].As<int>());
        SetType(m_type);
        m_texture->Load(parser);

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
        m_texture->Send(this);
        

        Wrapper::Renderer::GetInstance()->SetCubemapParameters();
    }

    void Resource::Cubemap::Save() const
    {
        CppSer::Serializer serializer(p_fileInfo.GetFullPath());
        serializer.SetVersion("1.0");
        serializer << CppSer::Pair::BeginMap << "Cubemap";
        serializer << CppSer::Pair::Key << "Type" << CppSer::Pair::Value << static_cast<int>(m_type);
        m_texture->Save(serializer);

        serializer << CppSer::Pair::EndMap << "Cubemap";
    }

    Weak<Resource::Cubemap> Resource::Cubemap::Create(const Path& path)
    {
        auto cubemap = Resource::ResourceManager::AddResource<Cubemap>(path);
        cubemap.lock()->Save();
        return cubemap;
    }

    void Resource::Cubemap::SetType(CubemapType type)
    {
        m_type = type;
        switch (type)
        {
        case CubemapType::Default:
            m_texture = std::make_unique<CubemapTexture>();
            break;
        case CubemapType::SixSided:
            m_texture = std::make_unique<SixSidedTexture>();
            break;
        case CubemapType::Panoramic:
            PrintError("Cubemap Type not implemented : %s", p_fileInfo.GetFullPath().string().c_str());
            break;
        default:
            PrintError("Unknown Cubemap Type : %s", p_fileInfo.GetFullPath().string().c_str());
            break;
        }
    }

    void Resource::Cubemap::ShowInInspector()
    {
        if (p_shouldBeLoaded && !p_loaded)
            return;
        m_texture->ShowOnInspector();
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
