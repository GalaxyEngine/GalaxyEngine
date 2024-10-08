#include "pch.h"
#include "Component/PointLight.h"
#include "Component/Transform.h"

#include "Resource/Shader.h"


namespace GALAXY
{
	void Component::PointLight::SendLightValues(Resource::Shader* shader)
	{
		Light::SendLightValues(shader);

		p_dirty |= GetTransform()->WasDirty();

		if (!p_dirty)
			return;

		p_position.value = GetTransform()->GetWorldPosition();
		shader->SendVec3f(p_position.string.c_str(), p_position.value);
		shader->SendFloat(p_constant.string.c_str(), p_constant.value);
		shader->SendFloat(p_linear.string.c_str(), p_linear.value);
		shader->SendFloat(p_quadratic.string.c_str(), p_quadratic.value);

		p_dirty = false;
	}

	void Component::PointLight::ShowInInspector()
	{
#ifdef WITH_EDITOR
		Light::ShowInInspector();

		ImGui::TextUnformatted("Attenuation");
		ImGui::TreePush("Attenuation");
		p_dirty |= ImGui::DragFloat("Constant", &p_constant.value, 0.01f);
		p_dirty |= ImGui::DragFloat("Linear", &p_linear.value, 0.01f);
		p_dirty |= ImGui::DragFloat("Quadratic", &p_quadratic.value, 0.01f);
		ImGui::TreePop();
#endif
	}

	void Component::PointLight::ComputeLocationName()
	{
		Light::ComputeLocationName();

		const std::string indexString = std::to_string(p_lightIndex);
		const std::string prefixString = "points[" + indexString;

		p_enableString = prefixString + "].enable";
		p_ambient.string = prefixString + "].ambient";
		p_diffuse.string = prefixString + "].diffuse";
		p_specular.string = prefixString + "].specular";

		p_position.string = prefixString + "].position";
		p_constant.string = prefixString + "].constant";
		p_linear.string = prefixString + "].linear";
		p_quadratic.string = prefixString + "].quadratic";
	}

	void Component::PointLight::Serialize(CppSer::Serializer& serializer)
	{
		Light::Serialize(serializer);

		serializer << CppSer::Pair::Key << "Constant" << CppSer::Pair::Value << p_constant.value;
		serializer << CppSer::Pair::Key << "Linear" << CppSer::Pair::Value << p_linear.value;
		serializer << CppSer::Pair::Key << "Quadratic" << CppSer::Pair::Value << p_quadratic.value;
	}

	void Component::PointLight::Deserialize(CppSer::Parser& parser)
	{
		Light::Deserialize(parser);

		p_constant.value = parser["Constant"].As<float>();
		p_linear.value = parser["Linear"].As<float>();
		p_quadratic.value = parser["Quadratic"].As<float>();
	}

}
