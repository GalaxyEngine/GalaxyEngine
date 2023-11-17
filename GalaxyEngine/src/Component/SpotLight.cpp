#include "pch.h"
#include "Component/SpotLight.h"
#include "Component/Transform.h"

#include "Resource/Shader.h"

#include "Utils/Parser.h"

namespace GALAXY 
{

	void Component::SpotLight::SendLightValues(Resource::Shader* shader)
	{
		bool dirty = p_dirty;

		PointLight::SendLightValues(shader);

		p_dirty |= GetTransform()->WasDirty();

		if (!dirty && !p_dirty)
			return;

		p_position.value = GetTransform()->GetWorldPosition();
		m_direction.value = GetTransform()->GetForward();
		shader->SendVec3f(p_position.string.c_str(), p_position.value);
		shader->SendVec3f(m_direction.string.c_str(), m_direction.value);
		shader->SendFloat(m_cutOff.string.c_str(), m_cutOff.value);
		shader->SendFloat(m_outerCutOff.string.c_str(), m_outerCutOff.value);

		p_dirty = false;
	}

	void Component::SpotLight::OnEditorDraw()
	{
		Vec3f worldPosition = GetTransform()->GetWorldPosition();
		Vec3f rayEndPosition = worldPosition + m_direction.value;
		Wrapper::Renderer::GetInstance()->DrawLine(worldPosition, rayEndPosition, Vec4f(0, 1, 1, 1), 5.f);
	}

	void Component::SpotLight::ShowInInspector()
	{
		PointLight::ShowInInspector();

		ImGui::TextUnformatted("Parameters");
		ImGui::TreePush("Parameters");

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4f /* = 1 / 2.5f */);
		p_dirty |= ImGui::DragFloat("CutOff Angle", &m_cutOff.value);
		p_dirty |= ImGui::DragFloat("Outer CutOff Angle", &m_outerCutOff.value);
		ImGui::PopItemWidth();

		ImGui::TreePop();
	}

	void Component::SpotLight::Serialize(Utils::Serializer& serializer)
	{
		PointLight::Serialize(serializer);

		serializer << Utils::PAIR::KEY << "CutOff" << Utils::PAIR::VALUE << m_cutOff.value;
		serializer << Utils::PAIR::KEY << "OuterCutOff" << Utils::PAIR::VALUE << m_outerCutOff.value;
	}

	void Component::SpotLight::Deserialize(Utils::Parser& parser)
	{
		PointLight::Deserialize(parser);

		m_cutOff.value = parser["CutOff"].As<float>();
		m_outerCutOff.value = parser["OuterCutOff"].As<float>();

		SetDirty();
	}

	void Component::SpotLight::ComputeLocationName()
	{
		Light::ComputeLocationName();

		std::string indexString = std::to_string(p_index);
		std::string prefixString = "spots[" + indexString;

		p_enableString = prefixString + "].enable";
		p_ambient.string = prefixString + "].ambient";
		p_diffuse.string = prefixString + "].diffuse";

		p_position.string = prefixString + "].position";
		p_constant.string = prefixString + "].constant";
		p_linear.string = prefixString + "].linear";
		p_quadratic.string = prefixString + "].quadratic";

		m_direction.string = prefixString + "].direction";
		m_cutOff.string = prefixString + "].cutOff";
		m_outerCutOff.string = prefixString + "].outerCutOff";
	}

}
