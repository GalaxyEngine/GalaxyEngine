#include "pch.h"
#include "Component/PointLight.h"
#include "Component/Transform.h"

#include "Resource/Shader.h"

#include "Utils/Parser.h"
namespace GALAXY 
{
	void Component::PointLight::SendLightValues(Resource::Shader* shader)
	{
		Light::SendLightValues(shader);

		shader->SendVec3f(m_positionString.c_str(), GetTransform()->GetWorldPosition());
		shader->SendFloat(m_constant.string.c_str(), m_constant.value);
		shader->SendFloat(m_linear.string.c_str(), m_linear.value);
		shader->SendFloat(m_quadratic.string.c_str(), m_quadratic.value);
	}

	void Component::PointLight::ShowInInspector()
	{
		Light::ShowInInspector();

		ImGui::TextUnformatted("Attenuation");
		ImGui::TreePush("Attenuation");
		ImGui::DragFloat("Constant", &m_constant.value, 0.01f);
		ImGui::DragFloat("Linear", &m_linear.value, 0.01f);
		ImGui::DragFloat("Quadratic", &m_quadratic.value, 0.01f);
		ImGui::TreePop();
	}

	void Component::PointLight::SetIndex(size_t val)
	{
		p_index = val;

		std::string indexString = std::to_string(p_index);
		std::string prefixString = "points[" + indexString;

		p_enableString = prefixString + "].enable";
		p_ambient.string = prefixString + "].ambient";
		p_diffuse.string = prefixString + "].diffuse";
		m_positionString = prefixString + "].position";
		p_specular.string = prefixString + "].specular";
		m_constant.string = prefixString + "].constant";
		m_linear.string = prefixString + "].linear";
		m_quadratic.string = prefixString + "].quadratic";
	}

	void Component::PointLight::Serialize(Utils::Serializer& serializer)
	{
		serializer << Utils::PAIR::KEY << "Constant" << Utils::PAIR::VALUE << m_constant.value;
		serializer << Utils::PAIR::KEY << "Linear" << Utils::PAIR::VALUE << m_linear.value;
		serializer << Utils::PAIR::KEY << "Quadratic" << Utils::PAIR::VALUE << m_quadratic.value;
	}

	void Component::PointLight::Deserialize(Utils::Parser& parser)
	{
		m_constant.value = parser["Constant"].As<float>();
		m_linear.value = parser["Linear"].As<float>();
		m_quadratic.value = parser["Quadratic"].As<float>();
	}

}
