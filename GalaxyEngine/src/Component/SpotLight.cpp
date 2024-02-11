#include "pch.h"

#include "Component/SpotLight.h"
#include "Component/Transform.h"

#include "Resource/Shader.h"



#include "Core/GameObject.h"

namespace GALAXY 
{

	void Component::SpotLight::SendLightValues(Resource::Shader* shader)
	{
		bool dirty = p_dirty;

		PointLight::SendLightValues(shader);

		p_dirty |= GetTransform()->WasDirty();

		if (!dirty && !p_dirty)
			return;

		m_direction.value = GetTransform()->GetForward();
		shader->SendVec3f(m_direction.string.c_str(), m_direction.value);
		shader->SendFloat(m_cutOff.string.c_str(), std::cos(DegToRad * m_cutOff.value));
		shader->SendFloat(m_outerCutOff.string.c_str(), std::cos(DegToRad * m_outerCutOff.value));

		p_dirty = false;
	}

	void Component::SpotLight::OnEditorDraw()
	{
		Light::OnEditorDraw();

		if (!GetGameObject()->IsSelected())
			return;
		static auto renderer = Wrapper::Renderer::GetInstance();
		Vec3f worldPosition = GetTransform()->GetWorldPosition();
		Quat worldRotation = GetTransform()->GetWorldRotation();
		renderer->DrawWireCone(worldPosition, worldRotation, 0.01f, m_outerCutOff, 25.f, Vec4f(0.980f, 0.804f, 0.0196f, 1.0f), 5.f);
		renderer->DrawWireCone(worldPosition, worldRotation, 0.01f, m_cutOff, 25.f, Vec4f(1, 1, 0, 1), 5.f);

		/* TODO: 
		*  Option to draw debug icon / debug line / other...
		*/
	}

	void Component::SpotLight::ShowInInspector()
	{
		PointLight::ShowInInspector();

		ImGui::TextUnformatted("Parameters");
		ImGui::TreePush("Parameters");

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4f /* = 1 / 2.5f */);
		ImGui::Text("Direction %s", m_direction.value.ToString().c_str());
		p_dirty |= ImGui::DragFloat("CutOff Angle", &m_cutOff.value);
		p_dirty |= ImGui::DragFloat("Outer CutOff Angle", &m_outerCutOff.value);
		ImGui::PopItemWidth();

		ImGui::TreePop();
	}

	void Component::SpotLight::Serialize(CppSer::Serializer& serializer)
	{
		PointLight::Serialize(serializer);

		serializer << CppSer::Pair::Key << "CutOff" << CppSer::Pair::Value << m_cutOff.value;
		serializer << CppSer::Pair::Key << "OuterCutOff" << CppSer::Pair::Value << m_outerCutOff.value;
	}

	void Component::SpotLight::Deserialize(CppSer::Parser& parser)
	{
		PointLight::Deserialize(parser);

		m_cutOff.value = parser["CutOff"].As<float>();
		m_outerCutOff.value = parser["OuterCutOff"].As<float>();

		SetDirty();
	}

	void Component::SpotLight::ComputeLocationName()
	{
		Light::ComputeLocationName();

		std::string indexString = std::to_string(p_lightIndex);
		std::string prefixString = "spots[" + indexString;

		p_enableString = prefixString + "].enable";
		p_ambient.string = prefixString + "].ambient";
		p_diffuse.string = prefixString + "].diffuse";
		p_specular.string = prefixString + "].specular";

		p_position.string = prefixString + "].position";
		p_constant.string = prefixString + "].constant";
		p_linear.string = prefixString + "].linear";
		p_quadratic.string = prefixString + "].quadratic";

		m_direction.string = prefixString + "].direction";
		m_cutOff.string = prefixString + "].cutOff";
		m_outerCutOff.string = prefixString + "].outerCutOff";
	}

}
