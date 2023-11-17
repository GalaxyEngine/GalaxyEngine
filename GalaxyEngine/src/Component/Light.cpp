#include "pch.h"
#include "Component/Light.h"

#include "Resource/Shader.h"

#include "Render/LightManager.h"

#include "Core/GameObject.h"

#include "Utils/Parser.h"

namespace GALAXY 
{

	void Component::Light::OnCreate()
	{
		if (!Render::LightManager::AddLight(GetGameObject()->GetWeakOfComponent(this)))
		{
			RemoveFromGameObject();
		}
	}

	void Component::Light::OnDestroy()
	{
		auto weak_this = GetGameObject()->GetWeakOfComponent(this);
		Render::LightManager::RemoveLight(weak_this);
	}

	void Component::Light::ShowInInspector()
	{
		ImGui::ColorEdit4("Ambient", m_ambient.Data());
		ImGui::ColorEdit4("Diffuse", m_diffuse.Data());
		ImGui::ColorEdit4("Specular", m_specular.Data());
	}

	void Component::Light::Serialize(Utils::Serializer& serializer)
	{
		serializer << Utils::PAIR::KEY << "Ambient" << Utils::PAIR::VALUE << m_ambient;
		serializer << Utils::PAIR::KEY << "Diffuse" << Utils::PAIR::VALUE << m_diffuse;
		serializer << Utils::PAIR::KEY << "Specular" << Utils::PAIR::VALUE << m_specular;
	}

	void Component::Light::Deserialize(Utils::Parser& parser)
	{
		m_ambient = parser["Ambient"].As<Vec4f>();
		m_diffuse = parser["Diffuse"].As<Vec4f>();
		m_specular = parser["Specular"].As<Vec4f>();
	}

	void Component::Light::SendLightValues(Resource::Shader* shader)
	{
	}

	void Component::Light::ResetLightValues(Resource::Shader* shader)
	{
	}

}
