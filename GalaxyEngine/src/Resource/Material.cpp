#include "pch.h"
#include "Resource/Material.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

namespace GALAXY {

	Resource::Material::Material(const std::filesystem::path& fullPath) : IResource(fullPath)
	{
		m_shader = Resource::ResourceManager::GetInstance()->GetDefaultShader();
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
				if (ImGui::Button("Set Texture", {ImGui::GetContentRegionAvail().x, 0}))
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

	void Resource::Material::SendValues()
	{
		auto renderer = Wrapper::Renderer::GetInstance();
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