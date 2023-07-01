#include "pch.h"
#include "Resource/Material.h"
#include "Resource/Texture.h"

namespace GALAXY {
	void Resource::Material::ShowInInspector()
	{
		if (ImGui::CollapsingHeader(p_name.c_str()))
		{
			ImGui::ColorEdit4("Ambient", &m_ambient.x);
			ImGui::ColorEdit4("Diffuse", &m_diffuse.x);
			ImGui::ColorEdit4("Specular", &m_specular.x);
			Wrapper::GUI::TextureButton(m_albedo.lock().get(), 32);
		}
	}
}