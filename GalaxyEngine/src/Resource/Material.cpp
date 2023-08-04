#include "pch.h"
#include "Resource/Material.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

namespace GALAXY {

	Resource::Material::Material(const std::string& fullPath) : IResource(fullPath)
	{
		m_shader = Resource::ResourceManager::GetInstance()->GetDefaultShader();
	}

	void Resource::Material::ShowInInspector()
	{
		if (ImGui::CollapsingHeader(p_name.c_str()))
		{
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
				ImGui::TextUnformatted(texture->GetName().c_str());
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