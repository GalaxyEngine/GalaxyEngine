#include "pch.h"
#include "Resource/Material.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

namespace GALAXY {

	Resource::Material::Material(const Path& fullPath) : IResource(fullPath)
	{
		m_shader = Resource::ResourceManager::GetInstance()->GetDefaultShader();
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
			bool sucess = LoadMatFile();
			if (!sucess)
				return;
		}

		p_loaded = true;
	}

	bool Resource::Material::LoadMatFile()
	{
		std::ifstream matFile(p_fileInfo.GetFullPath());
		if (!matFile.is_open())
		{
			PrintError("Failed to load Material %s", p_fileInfo.GetFullPath().string().c_str());
			return false;
		}

		std::string line;
		while (std::getline(matFile, line)) {
			size_t pos = line.find(" : ");
			if (pos != std::string::npos) {
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 3);

				if (key == "Shader") {
					if (value != NONE_RESOURCE)
						m_shader = ResourceManager::GetOrLoad<Shader>(value);
				}
				else if (key == "Albedo") {
					if (value != NONE_RESOURCE)
						m_albedo = ResourceManager::GetOrLoad<Texture>(value);
				}
				else if (key == "Normal") {
					if (value != NONE_RESOURCE)
						m_normal = ResourceManager::GetOrLoad<Texture>(value);
				}
				else if (key == "Ambient") {
					std::istringstream iss(value);
					iss >> m_ambient.x >> m_ambient.y >> m_ambient.z >> m_ambient.w;
				}
				else if (key == "Diffuse") {
					std::istringstream iss(value);
					iss >> m_diffuse.x >> m_diffuse.y >> m_diffuse.z >> m_diffuse.w;
				}
				else if (key == "Specular") {
					std::istringstream iss(value);
					iss >> m_specular.x >> m_specular.y >> m_specular.z >> m_specular.w;
				}
			}
		}

		matFile.close();
		return true;
	}

	bool Resource::Material::LoadMTLFile()
	{
		return false;
	}

	void Resource::Material::Save()
	{
		std::ofstream matFile(p_fileInfo.GetFullPath());
		if (matFile.is_open())
		{
			matFile << "Shader : " << (m_shader.lock() ? m_shader.lock()->GetFileInfo().GetRelativePath().generic_string() : NONE_RESOURCE) + '\n';
			matFile << "Albedo : " << (m_albedo.lock() ? m_albedo.lock()->GetFileInfo().GetRelativePath().generic_string() : NONE_RESOURCE) + '\n';
			matFile << "Normal : " << (m_normal.lock() ? m_normal.lock()->GetFileInfo().GetRelativePath().generic_string() : NONE_RESOURCE) + '\n';
			matFile << "Ambient : " << m_ambient << '\n';
			matFile << "Diffuse : " << m_diffuse << '\n';
			matFile << "Specular : " << m_specular << '\n';
			matFile.close();
		}
	}

	void Resource::Material::ShowInInspector()
	{
		if (ImGui::CollapsingHeader(GetName().c_str()))
		{
			if (ImGui::Button(m_shader.lock() ? m_shader.lock()->GetName().c_str() : "Set Shader"))
			{
				ImGui::OpenPopup("ShaderPopup");
			}
			if (auto sha = Resource::ResourceManager::GetInstance()->ResourcePopup<Resource::Shader>("ShaderPopup", { Resource::ResourceType::Shader, Resource::ResourceType::PostProcessShader }); sha.lock())
			{
				m_shader = sha;
			}
			ImGui::ColorEdit4("Ambient", &m_ambient.x);
			ImGui::ColorEdit4("Diffuse", &m_diffuse.x);
			ImGui::ColorEdit4("Specular", &m_specular.x);
			if (auto texture = m_albedo.lock()) {
				if (Wrapper::GUI::TextureButton(m_albedo.lock().get(), 32))
				{
					ImGui::OpenPopup("TexturePopup");
				}
				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::TextUnformatted(GetName().c_str());
				if (ImGui::Button("Clear"))
				{
					m_albedo.reset();
				}
				ImGui::EndGroup();
			}
			else
			{
				if (ImGui::Button("Set Texture", { ImGui::GetContentRegionAvail().x, 0 }))
				{
					ImGui::OpenPopup("TexturePopup");
				}
			}
			if (auto tex = Resource::ResourceManager::GetInstance()->ResourcePopup<Resource::Texture>("TexturePopup"); tex.lock())
			{
				m_albedo = tex;
			}
		}
	}

	Weak<Resource::Shader> Resource::Material::SendValues(uint64_t id /*= -1*/)
	{
		static auto renderer = Wrapper::Renderer::GetInstance();
		auto renderType = renderer->GetRenderType();
		Shared<Resource::Shader> shader = {};
		switch (renderType)
		{
		case Render::RenderType::DEFAULT:
		{
			shader = m_shader.lock();
			if (!shader || !shader->HasBeenSent())
				return {};
			shader->Use();

			shader->SendInt("EnableTexture", m_albedo.lock() ? true : false);
			if (auto texture = m_albedo.lock()) {
				texture->Bind(0);
				shader->SendInt("Texture", 0);
			}
			shader->SendVec4f("Diffuse", m_diffuse);
		}
		break;
		case Render::RenderType::PICKING:
		{
			shader = m_shader.lock()->GetPickingVariant().lock();
			if (!shader || !shader->HasBeenSent())
				return {};
			shader->Use();

			int r = (id & 0x000000FF) >> 0;
			int g = (id & 0x0000FF00) >> 8;
			int b = (id & 0x00FF0000) >> 16;

			shader->SendVec4f("Diffuse", Vec4f(r / 255.f, g / 255.f, b / 255.f, 1.f));
		}
		break;
		case Render::RenderType::OUTLINE:
		{
			shader = Resource::ResourceManager::GetInstance()->GetUnlitShader().lock();
			if (!shader || !shader->HasBeenSent())
				return {};
			shader->Use();

			shader->SendInt("EnableTexture", false);
			shader->SendVec4f("Diffuse", Vec4f(1));
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
		material.Save();
		return Resource::ResourceManager::GetInstance()->GetOrLoad<Material>(path);
	}

}