#include "pch.h"
#include "Component/DirectionalLight.h"

#include "Resource/Shader.h"

#include "Core/GameObject.h"

namespace GALAXY
{

	void Component::DirectionalLight::SendLightValues(Resource::Shader* shader)
	{
		Light::SendLightValues(shader);

		p_dirty |= GetTransform()->WasDirty();

		if (!p_dirty)
			return;

		m_direction.value = GetTransform()->GetForward();
		shader->SendVec3f(m_direction.string.c_str(), m_direction.value);

		p_dirty = false;
	}

	void Component::DirectionalLight::ShowInInspector()
	{
		Light::ShowInInspector();

		ImGui::Text("%s", m_direction.value.ToString().c_str());
	}

	void Component::DirectionalLight::OnEditorDraw()
	{
		Vec3f worldPosition = GetTransform()->GetWorldPosition();
		Vec3f rayEndPosition = worldPosition + m_direction.value;
		Wrapper::Renderer::GetInstance()->DrawLine(worldPosition, rayEndPosition, Vec4f(1, 1, 0, 1), 5.f);
	}

	void Component::DirectionalLight::ComputeLocationName()
	{
		Light::ComputeLocationName();

		std::string indexString = std::to_string(p_lightIndex);
		std::string prefixString = "directionals[" + indexString;

		p_enableString = prefixString + "].enable";
		p_ambient.string = prefixString + "].ambient";
		p_diffuse.string = prefixString + "].diffuse";
		p_specular.string = prefixString + "].specular";

		m_direction.string = prefixString + "].direction";
	}

}
