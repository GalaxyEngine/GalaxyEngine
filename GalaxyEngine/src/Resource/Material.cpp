#include "pch.h"
#include "Resource/Material.h"

#include "Core/Application.h"

#include "Editor/ThumbnailCreator.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

namespace GALAXY {

	Resource::Material::Material(const Path& fullPath) : IResource(fullPath)
	{
	}

	void Resource::Material::Load()
	{
		m_shader = Resource::ResourceManager::GetInstance()->GetDefaultShader();
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

		CreateDataFile();

		if (Editor::ThumbnailCreator::IsThumbnailUpToDate(this))
			return;
		CreateThumbnail();
	}

	void Resource::Material::OnAdd()
	{
	}

	void Resource::Material::CreateThumbnail()
	{
		static Editor::ThumbnailCreator* thumbnailCreator = Core::Application::GetInstance().GetThumbnailCreator();

		const Weak materialWeak = std::dynamic_pointer_cast<Material>(shared_from_this());

		thumbnailCreator->AddToQueue(materialWeak);
	}

	bool Resource::Material::LoadMatFile()
	{
		CppSer::Parser parser(p_fileInfo.GetFullPath());
		if (!parser.IsFileOpen())
			return false;
		m_shader = ResourceManager::GetOrLoad<Shader>(parser["Shader"].As<uint64_t>());
		m_albedo = ResourceManager::GetOrLoad<Texture>(parser["Albedo"].As<uint64_t>());
		m_normalMap = ResourceManager::GetOrLoad<Texture>(parser["Normal"].As<uint64_t>());
		m_parallaxMap = ResourceManager::GetOrLoad<Texture>(parser["Parallax"].As<uint64_t>());
		m_heightScale = parser["Height_Scale"].As<float>();
		m_ambient = parser["Ambient"].As<Vec4f>();
		m_diffuse = parser["Diffuse"].As<Vec4f>();
		m_specular = parser["Specular"].As<Vec4f>();

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
		SerializeResource(serializer, "Albedo", m_albedo);
		SerializeResource(serializer, "Normal", m_normalMap);
		SerializeResource(serializer, "Parallax", m_parallaxMap);

		serializer << CppSer::Pair::Key << "Height_Scale" << CppSer::Pair::Value << m_heightScale;
		serializer << CppSer::Pair::Key << "Ambient" << CppSer::Pair::Value << m_ambient;
		serializer << CppSer::Pair::Key << "Diffuse" << CppSer::Pair::Value << m_diffuse;
		serializer << CppSer::Pair::Key << "Specular" << CppSer::Pair::Value << m_specular;

		CreateThumbnail();
	}

	void Resource::Material::ShowInInspector()
	{
		if (ImGui::CollapsingHeader(GetName().c_str()))
		{
			if (ImGui::Button(m_shader.lock() ? m_shader.lock()->GetName().c_str() : "Set Shader"))
			{
				ImGui::OpenPopup("ShaderPopup");
			}
			Weak<Shader> sha;
			if (Resource::ResourceManager::GetInstance()->ResourcePopup("ShaderPopup", sha, { Resource::ResourceType::Shader, Resource::ResourceType::PostProcessShader }); sha.lock())
			{
				m_shader = sha;
			}
			ImGui::ColorEdit4("Ambient", &m_ambient.x);
			ImGui::ColorEdit4("Diffuse", &m_diffuse.x);
			ImGui::ColorEdit4("Specular", &m_specular.x);

			DisplayTexture("Set Albedo", m_albedo);
			DisplayTexture("Set Normal Map", m_normalMap);
			DisplayTexture("Set Parallax Map", m_parallaxMap);
			if (m_parallaxMap.lock())
			{
				ImGui::DragFloat("Height Scale", &m_heightScale, 0.1f);
			}

			if (ImGui::Button("Save"))
			{
				Save();
			}
		}
	}

	void Resource::Material::DisplayTexture(const char* textureLabel, Weak<Texture>& textureRef) const
	{
		ImGui::PushID(textureLabel);
		if (const auto texture = textureRef.lock()) {
			if (Wrapper::GUI::TextureButton(texture.get(), Vec2f(32)))
			{
				ImGui::OpenPopup("TexturePopup");
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(GetName().c_str());
		}
		else
		{
			if (ImGui::Button(textureLabel, { ImGui::GetContentRegionAvail().x, 0 }))
			{
				ImGui::OpenPopup("TexturePopup");
			}
		}
		Weak<Texture> tex;
		if (Resource::ResourceManager::GetInstance()->ResourcePopup("TexturePopup", tex))
		{
			textureRef = tex;
		}
		ImGui::PopID();
	}

	Weak<Resource::Shader> Resource::Material::SendValues(const uint64_t id /*= -1*/) const
	{
		static auto renderer = Wrapper::Renderer::GetInstance();
		const auto renderType = renderer->GetRenderType();
		Shared<Resource::Shader> shader = {};
		switch (renderType)
		{
		case Render::RenderType::Default:
		{
			shader = m_shader.lock();
			if (!shader || !shader->HasBeenSent())
				return {};
			shader->Use();

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
			static auto unlitShader = Resource::ResourceManager::GetInstance()->GetUnlitShader().lock();
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
		Material material(path);
		auto weakMat = Resource::ResourceManager::GetOrLoad<Material>(path);
		material.Save();
		return weakMat;
	}

}