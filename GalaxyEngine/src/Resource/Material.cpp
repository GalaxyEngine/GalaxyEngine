#include "pch.h"
#include "Resource/Material.h"

#include "Core/Application.h"

#ifdef WITH_EDITOR
#include "Editor/ThumbnailCreator.h"
#endif

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

namespace GALAXY
{
    Resource::Material::Material(const Path& fullPath) : IResource(fullPath)
    {
    }

    void Resource::Material::Load()
    {
        if (p_shouldBeLoaded)
            return;
        p_shouldBeLoaded = true;
        if (p_fileInfo.GetResourceType() == Resource::ResourceType::Materials)
        {
        }
        else
        {
            if (!LoadMatFile())
                return;
        }

        p_loaded = true;

        if (!std::filesystem::exists(GetDataFilePath()))
            CreateDataFile();

#ifdef WITH_EDITOR
        if (Editor::ThumbnailCreator::IsThumbnailUpToDate(this))
            return;
        CreateThumbnail();
#endif
    }

    void Resource::Material::OnAdd()
    {
    }

#ifdef WITH_EDITOR
    Path Resource::Material::GetThumbnailPath() const
    {
        return Editor::ThumbnailCreator::GetThumbnailPath(this);
    }
#endif

    typedef std::pair<std::string, CppSer::StringSerializer> StringPair;
    typedef std::unordered_map<std::string, CppSer::StringSerializer> StringsMap;
    typedef std::vector<StringsMap> StringMaps;

    template <typename Func>
    void ProcessDepth(CppSer::Parser& parser, const StringMaps& maps,  Func func)
    {
        parser.PushDepth();
        if (parser.GetCurrentDepth() < maps.size())
        {
            for (const StringPair& pair : maps[parser.GetCurrentDepth()])
            {
                func(pair);
            }
        }
    }

    bool Resource::Material::LoadMatFile()
    {
        CppSer::Parser parser(p_fileInfo.GetFullPath());
        if (!parser.IsFileOpen())
            return false;
        SetShader(ResourceManager::GetOrLoad<Shader>(parser["Shader"].As<uint64_t>()));
        if (!m_shader.lock())
            SetShader(Resource::ResourceManager::GetDefaultShader());

        /*
        if (parser.GetVersion() != "1.0")
        {
            ASSERT(false); // Debug
            PrintError("Invalid .mat file (maybe a previous version) : %s", p_fileInfo.GetFullPath().string().c_str());

            // Convert old .mat file to new one
            SetAlbedo(ResourceManager::GetOrLoad<Texture>(parser["Albedo"].As<uint64_t>()));
            SetNormalMap(ResourceManager::GetOrLoad<Texture>(parser["Normal"].As<uint64_t>()));
            SetParallaxMap(ResourceManager::GetOrLoad<Texture>(parser["Parallax"].As<uint64_t>()));
            SetHeightScale(parser["Height_Scale"].As<float>());
            SetAmbient(parser["Ambient"].As<Vec4f>());
            SetDiffuse(parser["Diffuse"].As<Vec4f>());
            SetSpecular(parser["Specular"].As<Vec4f>());
            Save();
            return true;
        }
        */

        parser.PushDepth();
        StringMaps maps = parser.GetValueMap();
        ProcessDepth(parser, maps, [this](const StringPair& pair) {SetBool(pair.first, pair.second.As<bool>()); });
        ProcessDepth(parser, maps, [this](const StringPair& pair) {SetInteger(pair.first, pair.second.As<int>()); });
        ProcessDepth(parser, maps, [this](const StringPair& pair) {SetFloat(pair.first, pair.second.As<float>()); });
        ProcessDepth(parser, maps, [this](const StringPair& pair) {SetColor(pair.first, pair.second.As<Vec4f>()); });
        ProcessDepth(parser, maps, [this](const StringPair& pair) {SetTexture(pair.first, ResourceManager::GetOrLoad<Texture>(pair.second.As<uint64_t>())); });
        ProcessDepth(parser, maps, [this](const StringPair& pair) {SetCubemap(pair.first, ResourceManager::GetOrLoad<Cubemap>(pair.second.As<uint64_t>())); });

        return true;
    }

    bool Resource::Material::LoadMTLFile()
    {
        return false;
    }

    void Resource::Material::Save()
    {
        CppSer::Serializer serializer(p_fileInfo.GetFullPath());
        serializer.SetVersion("1.0");
        serializer << CppSer::Pair::BeginMap << "Material";

        SerializeResource(serializer, "Shader", m_shader);

        serializer << CppSer::Pair::BeginMap << "Uniforms";
        serializer << CppSer::Pair::BeginTab;
        serializer << CppSer::Pair::BeginMap << "Bools";
        for (auto& pair : m_data.m_bools)
        {
            serializer << CppSer::Pair::Key << pair.first << CppSer::Pair::Value << pair.second;
        }
        serializer << CppSer::Pair::EndMap << "Bools";
        serializer << CppSer::Pair::BeginMap << "Ints";
        for (auto& pair : m_data.m_ints)
        {
            serializer << CppSer::Pair::Key << pair.first << CppSer::Pair::Value << pair.second;
        }
        serializer << CppSer::Pair::EndMap << "Ints";
        serializer << CppSer::Pair::BeginMap << "Floats";
        for (auto& pair : m_data.m_floats)
        {
            serializer << CppSer::Pair::Key << pair.first << CppSer::Pair::Value << pair.second;
        }
        serializer << CppSer::Pair::EndMap << "Floats";
        serializer << CppSer::Pair::BeginMap << "Float4s";
        for (auto& pair : m_data.m_float4)
        {
            serializer << CppSer::Pair::Key << pair.first << CppSer::Pair::Value << pair.second;
        }
        serializer << CppSer::Pair::EndMap << "Float4s";
        serializer << CppSer::Pair::BeginMap << "Textures";
        for (auto& pair : m_data.m_textures)
        {
            SerializeResource(serializer, pair.first.c_str(), pair.second);
        }
        serializer << CppSer::Pair::EndMap << "Textures";
        serializer << CppSer::Pair::BeginMap << "Cubemaps";
        for (auto& pair : m_data.m_cubemaps)
        {
            SerializeResource(serializer, pair.first.c_str(), pair.second);
        }
        serializer << CppSer::Pair::EndMap << "Cubemaps";
        serializer << CppSer::Pair::EndTab;
        serializer << CppSer::Pair::EndMap << "Uniforms";

        serializer << CppSer::Pair::EndMap << "Material";

#ifdef WITH_EDITOR
        CreateThumbnail();
#endif
    }

    void Resource::Material::ShowInInspector()
    {
#ifdef WITH_EDITOR
        if (ImGui::CollapsingHeader(GetName().c_str()))
        {
            if (Resource::ResourceManager::ResourceField(m_shader, "Shader"))
            {
                SetShader(m_shader);
            }
            /*
            ImGui::ColorEdit4("Ambient", &m_ambient.x);
            ImGui::ColorEdit4("Diffuse", &m_diffuse.x);
            ImGui::ColorEdit4("Specular", &m_specular.x);

            Resource::ResourceManager::ResourceField(m_albedo, "Albedo");
            Resource::ResourceManager::ResourceField(m_normalMap, "Normal Map");
            Resource::ResourceManager::ResourceField(m_parallaxMap, "Parallax Map");
            // DisplayTexture("Set Albedo", m_albedo);
            // DisplayTexture("Set Normal Map", m_normalMap);
            // DisplayTexture("Set Parallax Map", m_parallaxMap);
            if (m_parallaxMap.lock())
            {
                ImGui::DragFloat("Height Scale", &m_heightScale, 0.1f);
            }
            */

            for (auto& boolUniform : m_data.m_bools)
            {
                ImGui::Checkbox(boolUniform.first.c_str(), &boolUniform.second);
            }

            for (auto& intUniform : m_data.m_ints)
            {
                ImGui::DragInt(intUniform.first.c_str(), &intUniform.second, 1);
            }

            for (auto& floatUniform : m_data.m_floats)
            {
                ImGui::DragFloat(floatUniform.first.c_str(), &floatUniform.second, 0.01f);
            }

            for (auto& float4Uniform : m_data.m_float4)
            {
                ImGui::ColorEdit4(float4Uniform.first.c_str(), &float4Uniform.second.x, ImGuiColorEditFlags_None);
            }

            for (auto& textureUniform : m_data.m_textures)
            {
                if (ResourceManager::ResourceField(textureUniform.second, textureUniform.first))
                {
                    if (textureUniform.first == "albedo")
                        SetBool("hasAlbedo", true);
                    else if (textureUniform.first == "normalMap")
                        SetBool("hasNormalMap", true);
                    else if (textureUniform.first == "parallaxMap")
                        SetBool("hasParallaxMap", true);
                }
            }

            for (auto& cubemapUniform : m_data.m_cubemaps)
            {
                if (ResourceManager::ResourceField(cubemapUniform.second, cubemapUniform.first))
                {
                    
                }
            }

            if (ImGui::Button("Save"))
            {
                Save();
            }
        }
#endif
    }

    void Resource::Material::SendForDefault(Shared<Resource::Shader> shader) const
    {
        shader->Use();

        for (auto& boolUniform : m_data.m_bools)
        {
            shader->SendInt(("material." + boolUniform.first).c_str(), boolUniform.second);
        }

        for (auto& floatUniform : m_data.m_floats)
        {
            shader->SendFloat(("material." + floatUniform.first).c_str(), floatUniform.second);
        }

        for (auto& intUniform : m_data.m_ints)
        {
            shader->SendInt(("material." + intUniform.first).c_str(), intUniform.second);
        }

        for (auto& float4Uniform : m_data.m_float4)
        {
            shader->SendVec4f(("material." + float4Uniform.first).c_str(), float4Uniform.second);
        }

        uint32_t i = 0;
        for (auto& textureUniform : m_data.m_textures)
        {
            if (!textureUniform.second.lock()) continue;
            textureUniform.second.lock()->Bind(i);
            shader->SendInt(("material." + textureUniform.first).c_str(), i);
            i++;
        }

        i = 0;
        for (auto& cubemapUniform : m_data.m_cubemaps)
        {
            if (!cubemapUniform.second.lock()) continue;
            cubemapUniform.second.lock()->Bind(i);
            shader->SendInt(("material." + cubemapUniform.first).c_str(), i);
            i++;
        }

        /*
        shader->SendInt("material.hasAlbedo", m_albedo.lock() ? true : false);
        if (const Shared<Texture> texture = m_albedo.lock()) {
            texture->Bind(0);
            shader->SendInt("material.albedo", 0);
        }
        shader->SendInt("material.hasNormalMap", m_normalMap.lock() ? true : false);
        if (const Shared<Texture> texture = m_normalMap.lock()) {
            texture->Bind(1);
            shader->SendInt("material.normalMap", 1);
        }
        shader->SendInt("material.hasParallaxMap", m_parallaxMap.lock() ? true : false);
        if (const Shared<Texture> texture = m_parallaxMap.lock()) {
            texture->Bind(2);
            shader->SendInt("material.parallaxMap", 2);
            shader->SendFloat("material.heightScale", m_heightScale);
        }
        shader->SendVec4f("material.ambient", m_ambient);
        shader->SendVec4f("material.diffuse", m_diffuse);
        shader->SendVec4f("material.specular", m_specular);
        */
    }

    Weak<Resource::Shader> Resource::Material::SendValues(const uint64_t id /*= -1*/) const
    {
        auto renderer = Wrapper::Renderer::GetInstance();
        const auto renderType = renderer->GetRenderType();
        Shared<Resource::Shader> shader = {};
        switch (renderType)
        {
        case Render::RenderType::Default:
            {
                shader = m_shader.lock();
                if (!shader || !shader->HasBeenSent())
                    return {};
                SendForDefault(shader);
            }
            break;
        case Render::RenderType::Picking:
            {
                shader = m_shader.lock()->GetPickingVariant().lock();
                if (!shader || !shader->HasBeenSent())
                    return {};
                shader->Use();

                const int r = (id & 0x000000FF) >> 0;
                const int g = (id & 0x0000FF00) >> 8;
                const int b = (id & 0x00FF0000) >> 16;

                shader->SendVec4f("idColor", Vec4f(r / 255.f, g / 255.f, b / 255.f, 1.f));
            }
            break;
        case Render::RenderType::Outline:
            {
                auto unlitShader = Resource::ResourceManager::GetInstance()->GetUnlitShader().lock();
                if (!unlitShader || !unlitShader->HasBeenSent())
                    return {};
                unlitShader->Use();

                unlitShader->SendInt("material.hasAlbedo", false);
                unlitShader->SendVec4f("material.diffuse", Vec4f(1));
                return unlitShader;
            }
            break;
        default:
            break;
        }
        return shader;
    }

    Weak<Resource::Material> Resource::Material::Create(const std::filesystem::path& path)
    {
        auto material = Resource::ResourceManager::AddResource<Material>(path);
        material.lock()->SetShader(Resource::ResourceManager::GetInstance()->GetDefaultShader());
        material.lock()->Save();
        return material;
    }

    void Resource::Material::SetShader(const Weak<Shader>& val)
    {
        // ! Calling to much in a small amount of time can break the material, and set all there value to default 
        if (val.lock() == m_shader.lock())
            return;
        m_shader = val;

        m_data = MaterialData();

        if (val.lock()->HasBeenSent())
            OnShaderLoaded(shared_from_this(), val);
        else
            val.lock()->OnLoad.Bind(std::bind(&Material::OnShaderLoaded, shared_from_this(), val));
    }        

    
    void Resource::Material::OnShaderLoaded(const Weak<IResource>& material, const Weak<Shader>& shader)
    {
        auto mat = std::dynamic_pointer_cast<Material>(material.lock());
        if (!mat)
        {
            PrintError("Material expired");
            return;
        }

        if (shader.expired() || !mat->m_shader.lock()->HasBeenSent() || mat->m_shader.lock().get() != shader.lock().get())
            return;
        UMap<std::string, Uniform> uniformMap = mat->m_shader.lock()->GetUniforms();
        for (auto& uniformPair : uniformMap)
        {
            if (!uniformPair.second.shouldDisplay)
                continue;
            switch (uniformPair.second.type)
            {
            case UniformType::Float:
                mat->m_data.m_floats[uniformPair.second.displayName] = 0.0f;
                break;
            case UniformType::Float4:
                mat->m_data.m_float4[uniformPair.second.displayName] = Vec4f(0.0f, 0.0f, 0.0f, 0.0f);
                break;
            case UniformType::Int:
                mat->m_data.m_ints[uniformPair.second.displayName] = 0;
                break;
            case UniformType::Bool:
                mat->m_data.m_bools[uniformPair.second.displayName] = 0;
                break;
            case UniformType::Texture2D:
                mat->m_data.m_textures[uniformPair.second.displayName] = {};
                break;
            case UniformType::CubeMap:
                mat->m_data.m_cubemaps[uniformPair.second.displayName] = {};
                break;
            default:
                PrintError("Not supported uniform type %d", uniformPair.second.type);
                ASSERT(false);
            }
        }
        if (mat->m_tempData.m_bools.empty() && mat->m_tempData.m_floats.empty() && mat->m_tempData.m_ints.empty() && mat->m_tempData.
            m_float4.empty() && mat->m_tempData.m_textures.empty() && mat->m_tempData.m_cubemaps.empty())
        {
            return;
        }
        // Set the values stored in the emp data
        for (auto& bools : mat->m_tempData.m_bools)
        {
            if (mat->m_data.m_bools.contains(bools.first))
            {
                mat->m_data.m_bools[bools.first] = bools.second;
            }
        }
        for (auto& ints : mat->m_tempData.m_ints)
        {
            if (mat->m_data.m_ints.contains(ints.first))
            {
                mat->m_data.m_ints[ints.first] = ints.second;
            }
        }
        for (auto& floats : mat->m_tempData.m_floats)
        {
            if (mat->m_data.m_floats.contains(floats.first))
            {
                mat->m_data.m_floats[floats.first] = floats.second;
            }
        }
        for (auto& float4 : mat->m_tempData.m_float4)
        {
            if (mat->m_data.m_float4.contains(float4.first))
            {
                mat->m_data.m_float4[float4.first] = float4.second;
            }
        }
        for (auto& textures : mat->m_tempData.m_textures)
        {
            if (mat->m_data.m_textures.contains(textures.first))
            {
                mat->m_data.m_textures[textures.first] = textures.second;
            }
        }
        for (auto& cubemaps : mat->m_tempData.m_cubemaps)
        {
            if (mat->m_data.m_cubemaps.contains(cubemaps.first))
            {
                mat->m_data.m_cubemaps[cubemaps.first] = cubemaps.second;
            }
        }
        mat->m_tempData = MaterialData();
    }

    void Resource::Material::SetBool(const std::string& name, const bool val)
    {
        // Check if shader is valid, if yes check also if the name is in the map else create it because we don't know if we will need it
        if (IsShaderValid() && m_data.m_bools.contains(name))
            m_data.m_bools[name] = val;
        else if (!m_shader.lock()->HasBeenSent())
            m_tempData.m_bools[name] = val;
    }

    void Resource::Material::SetInteger(const std::string& name, const int val)
    {
        if (IsShaderValid() && m_data.m_ints.contains(name))
            m_data.m_ints[name] = val;
        else if (!m_shader.lock()->HasBeenSent())
            m_tempData.m_ints[name] = val;
    }

    void Resource::Material::SetFloat(const std::string& name, const float val)
    {
        if (IsShaderValid() && m_data.m_floats.contains(name))
            m_data.m_floats[name] = val;
        else if (!m_shader.lock()->HasBeenSent())
            m_tempData.m_floats[name] = val;
    }

    void Resource::Material::SetColor(const std::string& name, const Vec4f& val)
    {
        if (IsShaderValid() && m_data.m_float4.contains(name))
            m_data.m_float4[name] = val;
        else if (!m_shader.lock()->HasBeenSent())
            m_tempData.m_float4[name] = val;
    }

    void Resource::Material::SetTexture(const std::string& name, const Weak<Texture>& val)
    {
        if (IsShaderValid() && m_data.m_textures.contains(name))
            m_data.m_textures[name] = val;
        else if (!m_shader.lock()->HasBeenSent())
            m_tempData.m_textures[name] = val;
    }

    void Resource::Material::SetCubemap(const std::string& name, const Weak<Cubemap>& val)
    {
        if (IsShaderValid() && m_data.m_cubemaps.contains(name))
            m_data.m_cubemaps[name] = val;
        else if (!m_shader.lock()->HasBeenSent())
            m_tempData.m_cubemaps[name] = val;
    }

    bool Resource::Material::GetBool(const std::string& name) const
    {
        if (IsShaderValid() && m_data.m_bools.contains(name))
            return m_data.m_bools.at(name);
        else if (!m_shader.lock()->HasBeenSent() && m_tempData.m_bools.contains(name))
            return m_tempData.m_bools.at(name);
        else
        {
            PrintError("Can't find bool: %s in material %s", name.c_str(), this->GetName());
            return false;
        }
    }

    const UMap<std::string, bool>& Resource::Material::GetBools() const
    {
        if (IsShaderValid() || m_shader.expired())
            return m_data.m_bools;
        else
            return m_tempData.m_bools;
    }

    int Resource::Material::GetInteger(const std::string& name) const
    {
        if (IsShaderValid() && m_data.m_ints.contains(name))
            return m_data.m_ints.at(name);
        else if (!m_shader.lock()->HasBeenSent() && m_tempData.m_ints.contains(name))
            return m_tempData.m_ints.at(name);
        else
        {
            PrintError("Can't find integer: %s in material %s", name.c_str(), this->GetName());
            return -1;
        }
    }

    const UMap<std::string, int>& Resource::Material::GetIntegers() const
    {
        if (IsShaderValid() || m_shader.expired())
            return m_data.m_ints;
        else
            return m_tempData.m_ints;
    }

    float Resource::Material::GetFloat(const std::string& name) const
    {
        if (IsShaderValid() && m_data.m_floats.contains(name))
            return m_data.m_floats.at(name);
        else if (!m_shader.lock()->HasBeenSent() && m_tempData.m_floats.contains(name))
            return m_tempData.m_floats.at(name);
        else
        {
            PrintError("Can't find float: %s in material %s", name.c_str(), this->GetName());
            return 0.0f;
        }
    }

    const UMap<std::string, float>& Resource::Material::GetFloats() const
    {
        if (IsShaderValid() || m_shader.expired())
            return m_data.m_floats;
        else
            return m_tempData.m_floats;
    }

    Vec4f Resource::Material::GetColor(const std::string& name) const
    {
        if (IsShaderValid() && m_data.m_float4.contains(name))
            return m_data.m_float4.at(name);
        else if (!m_shader.lock()->HasBeenSent() && m_tempData.m_float4.contains(name))
            return m_tempData.m_float4.at(name);
        else
        {
            PrintError("Can't find color: %s in material %s", name.c_str(), this->GetName());
            return {0.0f, 0.0f, 0.0f, 0.0f};
        }
    }

    const UMap<std::string, Vec4f>& Resource::Material::GetColors() const
    {
        if (IsShaderValid() || m_shader.expired())
            return m_data.m_float4;
        else
            return m_tempData.m_float4;
    }

    Weak<Resource::Texture> Resource::Material::GetTexture(const std::string& name) const
    {
        if (IsShaderValid() && m_data.m_textures.contains(name))
            return m_data.m_textures.at(name);
        else if (!m_shader.lock()->HasBeenSent() && m_tempData.m_textures.contains(name))
            return m_tempData.m_textures.at(name);
        else
        {
            PrintError("Can't find texture: %s in material %s", name.c_str(), GetName());
            return {};
        }
    }

    const UMap<std::string, Weak<Resource::Texture>>& Resource::Material::GetTextures() const
    {
        if (IsShaderValid() || m_shader.expired())
            return m_data.m_textures;
        else
            return m_tempData.m_textures;
    }

    Weak<Resource::Cubemap> Resource::Material::GetCubemap(const std::string& name) const
    {
        if (IsShaderValid() && m_data.m_cubemaps.contains(name))
            return m_data.m_cubemaps.at(name);
        else if (!m_shader.lock()->HasBeenSent() && m_tempData.m_cubemaps.contains(name))
            return m_tempData.m_cubemaps.at(name);
        else
        {
            PrintError("Can't find cubemap: %s in material %s", name.c_str(), GetName());
            return {};
        }
    }

    const UMap<std::string, Weak<Resource::Cubemap>>& Resource::Material::GetCubemaps() const
    {
        if (IsShaderValid() || m_shader.expired())
            return m_data.m_cubemaps;
        else
            return m_tempData.m_cubemaps;
    }

#ifdef WITH_EDITOR
    void Resource::Material::CreateThumbnail()
    {
        Editor::ThumbnailCreator* thumbnailCreator = Core::Application::GetInstance().GetThumbnailCreator();

        const Weak materialWeak = std::dynamic_pointer_cast<Material>(shared_from_this());

        thumbnailCreator->AddToQueue(materialWeak);
    }
#endif

}
