#include "pch.h"
#include "Resource/Material.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

namespace GALAXY {

	Resource::Material::Material(const std::filesystem::path& fullPath) : IResource(fullPath)
	{
		m_shader = Resource::ResourceManager::GetInstance()->GetDefaultShader();
	}

	void Resource::Material::Load()
	{
		// TODO : Parse Material File
		std::ifstream matFile(p_fileInfo.GetFullPath());
		if (!matFile.is_open())
		{
			PrintError("Failed to load Material %s", p_fileInfo.GetFullPath().string().c_str());
			return;
		}

		std::string line;
		while (std::getline(matFile, line)) {
			size_t pos = line.find(" : ");
			if (pos != std::string::npos) {
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 3);

				if (key == "Shader") {
					m_shader = ResourceManager::GetOrLoad<Shader>(value);
				}
				else if (key == "Albedo") {
					m_albedo = ResourceManager::GetOrLoad<Texture>(value);
				}
				else if (key == "Normal") {
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
	}

	void Resource::Material::Save()
	{
		// TODO : Save Material File
		std::ofstream matFile(p_fileInfo.GetFullPath());
		if (matFile.is_open())
		{
			matFile << "Shader : " << (m_shader.lock() ? m_shader.lock()->GetFileInfo().GetRelativePath().string() : std::string("None")) + '\n';
			matFile << "Albedo : " << (m_albedo.lock() ? m_albedo.lock()->GetFileInfo().GetRelativePath().string() : std::string("None")) + '\n';
			matFile << "Normal : " << (m_normal.lock() ? m_normal.lock()->GetFileInfo().GetRelativePath().string() : std::string("None")) + '\n';
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
			if (auto sha = Resource::ResourceManager::GetInstance()->ResourcePopup<Resource::Shader>("ShaderPopup"); sha.lock())
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

	void Resource::Material::SendValues(uint64_t id /*= -1*/)
	{
		static auto renderer = Wrapper::Renderer::GetInstance();
		if (id != -1)
		{
			auto shader = m_shader.lock()->GetPickingVariant().lock();
			if (!shader || !shader->HasBeenSent())
				return;
			shader->Use();

			int r = (id & 0x000000FF) >> 0;
			int g = (id & 0x0000FF00) >> 8;
			int b = (id & 0x00FF0000) >> 16;
			
			renderer->ShaderSendVec4f(shader->GetLocation("Diffuse"), Vec4f(r / 255.f, g / 255.f, b / 255.f, 1.f));
		}
		else
		{
			auto shader = m_shader.lock();
			if (!shader || !shader->HasBeenSent())
				return;
			shader->Use();

			renderer->ShaderSendInt(shader->GetLocation("EnableTexture"), m_albedo.lock() ? true : false);
			if (auto texture = m_albedo.lock()) {
				texture->Bind(0);
				renderer->ShaderSendInt(shader->GetLocation("Texture"), 0);
			}
			renderer->ShaderSendVec4f(shader->GetLocation("Diffuse"), m_diffuse);
		}
	}

	Weak<Resource::Material> Resource::Material::Create(const std::filesystem::path& path)
	{
		Material material(path);
		material.Save();
		return Resource::ResourceManager::GetInstance()->GetOrLoad<Material>(path);
	}

}