#include "pch.h"
#include "Resource/Material.h"

#include "Core/Application.h"

#ifdef WITH_EDITOR
#include "Editor/ThumbnailCreator.h"
#endif

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

namespace GALAXY {

	Resource::Material::Material(const Path& fullPath) : IResource(fullPath)
	{
	}

	void Resource::Material::Load()
	{
		SetShader(Resource::ResourceManager::GetInstance()->GetDefaultShader());
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

	Path Resource::Material::GetThumbnailPath() const
	{
		return Editor::ThumbnailCreator::GetThumbnailPath(p_uuid);
	}

	bool Resource::Material::LoadMatFile()
	{
		CppSer::Parser parser(p_fileInfo.GetFullPath());
		if (!parser.IsFileOpen())
			return false;
		SetShader(ResourceManager::GetOrLoad<Shader>(parser["Shader"].As<uint64_t>()));
		if (!m_shader.lock())
			SetShader(Resource::ResourceManager::GetDefaultShader());

		if (parser.GetValueMap().size() < 7)
		{
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
		
		parser.PushDepth();
		parser.PushDepth();
		for (const auto& pair : parser.GetValueMap()[parser.GetCurrentDepth()])
		{
			m_bools[pair.first] = pair.second.As<bool>();
		}
		
		parser.PushDepth();
		for (const auto& pair : parser.GetValueMap()[parser.GetCurrentDepth()])
		{
			m_ints[pair.first] = pair.second.As<int>();
		}

		parser.PushDepth();
		for (const auto& pair : parser.GetValueMap()[parser.GetCurrentDepth()])
		{
			m_floats[pair.first] = pair.second.As<float>();
		}

		parser.PushDepth();
		for (const auto& pair : parser.GetValueMap()[parser.GetCurrentDepth()])
		{
			m_float4[pair.first] = pair.second.As<Vec4f>();
		}

		parser.PushDepth();
		for (const auto& pair : parser.GetValueMap()[parser.GetCurrentDepth()])
		{
			m_textures[pair.first] = ResourceManager::GetOrLoad<Texture>(pair.second.As<uint64_t>());
		}

		return true;
	}

	bool Resource::Material::LoadMTLFile()
	{
		return false;
	}

	void Resource::Material::Save()
	{
		CppSer::Serializer serializer(p_fileInfo.GetFullPath());
		serializer << CppSer::Pair::BeginMap << "Material";

		SerializeResource(serializer, "Shader", m_shader);

		serializer << CppSer::Pair::BeginMap << "Uniforms";
		serializer << CppSer::Pair::BeginTab;
		serializer << CppSer::Pair::BeginMap << "Bools";
		for (auto& pair : m_bools)
		{
			serializer << CppSer::Pair::Key << pair.first << CppSer::Pair::Value << pair.second;
		}
		serializer << CppSer::Pair::EndMap << "Bools";
		serializer << CppSer::Pair::BeginMap << "Ints";
		for (auto& pair : m_ints)
		{
			serializer << CppSer::Pair::Key << pair.first << CppSer::Pair::Value << pair.second;
		}
		serializer << CppSer::Pair::EndMap << "Ints";
		serializer << CppSer::Pair::BeginMap << "Floats";
		for (auto& pair : m_floats)
		{
			serializer << CppSer::Pair::Key << pair.first << CppSer::Pair::Value << pair.second;
		}
		serializer << CppSer::Pair::EndMap << "Floats";
		serializer << CppSer::Pair::BeginMap << "Float4s";
		for (auto& pair : m_float4)
		{
			serializer << CppSer::Pair::Key << pair.first << CppSer::Pair::Value << pair.second;
		}
		serializer << CppSer::Pair::EndMap << "Float4s";
		serializer << CppSer::Pair::BeginMap << "Textures";
		for (auto& pair : m_textures)
		{
			SerializeResource(serializer, pair.first.c_str(), pair.second);
		}
		serializer << CppSer::Pair::EndMap << "Textures";
		serializer << CppSer::Pair::EndTab;
		serializer << CppSer::Pair::EndMap << "Uniforms";

		serializer << CppSer::Pair::EndMap << "Material";

#ifdef WITH_EDITOR
		CreateThumbnail();
#endif
	}

	void Resource::Material::ShowInInspector()
	{
		if (ImGui::CollapsingHeader(GetName().c_str()))
		{
			Resource::ResourceManager::ResourceField(m_shader, "Shader");
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

			for (auto& boolUniform : m_bools)
			{
				ImGui::Checkbox(boolUniform.first.c_str(), &boolUniform.second);
			}

			for (auto& intUniform : m_ints)
			{
				ImGui::DragInt(intUniform.first.c_str(), &intUniform.second, 1);
			}

			for (auto& floatUniform : m_floats)
			{
				ImGui::DragFloat(floatUniform.first.c_str(), &floatUniform.second, 0.01f);
			}

			for (auto& float4Uniform : m_float4)
			{
				ImGui::ColorEdit4(float4Uniform.first.c_str(), &float4Uniform.second.x, ImGuiColorEditFlags_None);
			}

			for (auto& textureUniform : m_textures)
			{
				if (Resource::ResourceManager::ResourceField(textureUniform.second, textureUniform.first))
				{
					if (textureUniform.first == "albedo")
						SetBool("hasAlbedo", true);
					else if (textureUniform.first == "normalMap")
						SetBool("hasNormalMap", true);
					else if (textureUniform.first == "parallaxMap")
						SetBool("hasParallaxMap", true);
				}
			}

			if (ImGui::Button("Save"))
			{
				Save();
			}
		}
	}

	void Resource::Material::SendForDefault(Shared<Resource::Shader> shader) const
	{
		shader->Use();

		for (auto& boolUniform : m_bools)
		{
			shader->SendInt(("material." + boolUniform.first).c_str(), boolUniform.second);
		}

		for (auto& floatUniform : m_floats)
		{
			shader->SendFloat(("material." + floatUniform.first).c_str(), floatUniform.second);
		}

		for (auto& intUniform : m_ints)
		{
			shader->SendInt(("material." + intUniform.first).c_str(), intUniform.second);
		}

		for (auto& float4Uniform : m_float4)
		{
			shader->SendVec4f(("material." + float4Uniform.first).c_str(), float4Uniform.second);
		}

		uint32_t i = 0;
		for (auto& textureUniform : m_textures)
		{
			if (!textureUniform.second.lock()) continue;
			textureUniform.second.lock()->Bind(i);
			shader->SendInt(("material." + textureUniform.first).c_str(), i);
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

#ifdef WITH_EDITOR
	void Resource::Material::SetShader(const Weak<Shader>& val)
	{
		m_shader = val;

		m_bools.clear();
		m_floats.clear();
		m_ints.clear();
		m_float4.clear();
		m_textures.clear();

		auto lambda = [&]()
		{
			UMap<std::string, Uniform> uniformMap = val.lock()->GetUniforms();
			for (auto& uniformPair : uniformMap)
			{
				if (!uniformPair.second.shouldDisplay)
					continue;
				switch (uniformPair.second.type) {
				case UniformType::Float:
					m_floats[uniformPair.second.displayName] = 0.0f;
					break;
				case UniformType::Float4:
					m_float4[uniformPair.second.displayName] = Vec4f(0.0f, 0.0f, 0.0f, 0.0f);
					break;
				case UniformType::Int:
					m_ints[uniformPair.second.displayName] = 0;
					break;
				case UniformType::Bool:
					m_bools[uniformPair.second.displayName] = 0;
					break;
				case UniformType::Texture2D:
					m_textures[uniformPair.second.displayName] = {};
					break;
				}
			}
		};
		

		if (val.lock()->HasBeenSent())
			lambda();
		else
			val.lock()->OnLoad.Bind(lambda);
		
	}

	void Resource::Material::CreateThumbnail()
	{
		Editor::ThumbnailCreator* thumbnailCreator = Core::Application::GetInstance().GetThumbnailCreator();

		const Weak materialWeak = std::dynamic_pointer_cast<Material>(shared_from_this());

		thumbnailCreator->AddToQueue(materialWeak);
	}
#endif

}