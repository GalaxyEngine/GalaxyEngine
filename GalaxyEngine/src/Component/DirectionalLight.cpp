#include "pch.h"
#include "Component/DirectionalLight.h"

#include "Resource/Shader.h"

#include "Core/GameObject.h"

namespace GALAXY
{

	void Component::DirectionalLight::SendLightValues(Resource::Shader* shader)
	{
		Light::SendLightValues(shader);

		shader->SendInt(m_enableString.c_str(), IsEnable());
		shader->SendVec4f(m_ambientString.c_str(), m_ambient);
		shader->SendVec4f(m_diffuseString.c_str(), m_diffuse);
		shader->SendVec4f(m_specularString.c_str(), m_specular);
		shader->SendVec3f(m_directionString.c_str(), m_direction);
	}

	void Component::DirectionalLight::ResetLightValues(Resource::Shader* shader)
	{
		Light::ResetLightValues(shader);
		shader->SendInt(m_enableString.c_str(), false);
	}

	void Component::DirectionalLight::ShowInInspector()
	{
		Light::ShowInInspector();

		m_direction = GetGameObject()->GetTransform()->GetForward();
		ImGui::Text("%s", m_direction.ToString().c_str());
	}

	void Component::DirectionalLight::OnEditorDraw()
	{
		Vec3f worldPosition = GetTransform()->GetWorldPosition();
		Vec3f rayEndPosition = worldPosition + m_direction;
		Wrapper::Renderer::GetInstance()->DrawLine(worldPosition, rayEndPosition, Vec4f(1, 1, 0, 1), 5.f);
	}

	void Component::DirectionalLight::SetIndex(size_t val)
	{
		m_index = val;

		std::string indexString = std::to_string(m_index);
		std::string prefixString = "directionals[" + indexString;

		m_enableString = prefixString + "].enable";
		m_ambientString = prefixString + "].ambient";
		m_diffuseString = prefixString + "].diffuse";
		m_specularString = prefixString + "].specular";
		m_directionString = prefixString + "].direction";
	}

}
