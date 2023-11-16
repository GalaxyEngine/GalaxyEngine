#include "pch.h"
#include "Component/DirectionalLight.h"

#include "Resource/Shader.h"

#include "Core/GameObject.h"

namespace GALAXY 
{

	void Component::DirectionalLight::SendLightValues(Resource::Shader* shader)
	{
		Light::SendLightValues(shader);
		shader->SendInt("directional.enable", IsEnable());
		shader->SendVec4f("directional.ambient", m_ambient);
		shader->SendVec4f("directional.diffuse", m_diffuse);
		shader->SendVec4f("directional.specular", m_specular);
		shader->SendVec3f("directional.direction", m_direction);
	}

	void Component::DirectionalLight::ResetLightValues(Resource::Shader* shader)
	{
		Light::ResetLightValues(shader);
		shader->SendInt("directional.enable", false);
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

}
