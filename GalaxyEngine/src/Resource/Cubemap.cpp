#include "pch.h"
#include "Resource/Texture.h"
#include "Resource/Cubemap.h"

#include "Resource/ResourceManager.h"


namespace GALAXY 
{    
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

        size_t i = 0;
        for (const auto& pair : parser.GetValueMap()[parser.GetCurrentDepth()])
        {
            auto textureUUID = pair.second.As<uint64_t>();
            m_textures[i++] = ResourceManager::GetOrLoad<Texture>(textureUUID);
        }

        //TODO init with opengl cubemap
    }

    void Resource::Cubemap::Save() const
    {
		CppSer::Serializer serializer(p_fileInfo.GetFullPath());

        serializer << CppSer::Pair::BeginMap << "Cubemap";

        for (size_t i = 0; i < m_textures.size(); i++)
        {
            SerializeResource(serializer, GetDirectionFromIndex(i).c_str(), m_textures[i]);
        }
        
        serializer << CppSer::Pair::EndMap << "Cubemap";
        
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
