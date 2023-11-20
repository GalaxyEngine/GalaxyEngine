#include "pch.h"
#include "Component/Light.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"

#include "Render/LightManager.h"

#include "Core/GameObject.h"

#include "Utils/Define.h"
#include "Utils/Parser.h"

namespace GALAXY 
{

	void Component::Light::OnCreate()
	{
		if (!Render::LightManager::AddLight(GetGameObject()->GetWeakOfComponent(this)))
		{
			RemoveFromGameObject();
		}
		m_editorIcon.SetIconTexture(Resource::ResourceManager::GetOrLoad<Resource::Texture>(LIGHT_ICON_PATH));
	}

	void Component::Light::OnDestroy()
	{
		auto weak_this = GetGameObject()->GetWeakOfComponent(this);
		Render::LightManager::RemoveLight(weak_this);
	}

	void Component::Light::OnEditorDraw()
	{
		m_editorIcon.Render(GetTransform()->GetModelMatrix(), GetGameObject()->GetSceneGraphID());
	}

	void Component::Light::ShowInInspector()
	{
		p_dirty |= ImGui::ColorEdit4("Ambient", p_ambient.value.Data());
		p_dirty |= ImGui::ColorEdit4("Diffuse", p_diffuse.value.Data());
		p_dirty |= ImGui::ColorEdit4("Specular", p_specular.value.Data());
	}

	void Component::Light::Serialize(Utils::Serializer& serializer)
	{
		serializer << Utils::Pair::KEY << "Ambient" << Utils::Pair::VALUE << p_ambient.value;
		serializer << Utils::Pair::KEY << "Diffuse" << Utils::Pair::VALUE << p_diffuse.value;
		serializer << Utils::Pair::KEY << "Specular" << Utils::Pair::VALUE << p_specular.value;
	}

	void Component::Light::Deserialize(Utils::Parser& parser)
	{
		p_ambient.value = parser["Ambient"].As<Vec4f>();
		p_diffuse.value = parser["Diffuse"].As<Vec4f>();
		p_specular.value = parser["Specular"].As<Vec4f>();

		p_dirty = true;
	}

	void Component::Light::SendLightValues(Resource::Shader* shader)
	{
		// Always send boolean is enable
		shader->SendInt(p_enableString.c_str(), IsEnable());

		if (!p_dirty)
			return;

		shader->SendVec4f(p_ambient.string.c_str(), p_ambient.value);
		shader->SendVec4f(p_diffuse.string.c_str(), p_diffuse.value);
		shader->SendVec4f(p_specular.string.c_str(), p_specular.value);
	}

	void Component::Light::ResetLightValues(Resource::Shader* shader)
	{
		shader->SendInt(p_enableString.c_str(), false);
	}

	void Component::Light::ComputeLocationName()
	{
		SetDirty();
	}

}
