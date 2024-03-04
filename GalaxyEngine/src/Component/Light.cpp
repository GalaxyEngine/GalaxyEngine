#include "pch.h"
#include "Component/Light.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"

#include "Render/LightManager.h"

#include "Core/GameObject.h"

#include "Utils/Define.h"


namespace GALAXY 
{

	void Component::Light::OnCreate()
	{
		if (!Render::LightManager::AddLight(GetGameObject()->GetWeakOfComponent(this)))
		{
			RemoveFromGameObject();
		}
#ifdef WITH_EDITOR
		m_editorIcon.SetIconTexture(Resource::ResourceManager::GetOrLoad<Resource::Texture>(LIGHT_ICON_PATH));
#endif
	}

	void Component::Light::OnDestroy()
	{
		const Weak<Light> weak_this = GetGameObject()->GetWeakOfComponent(this);
		Render::LightManager::RemoveLight(weak_this);
	}

	void Component::Light::OnEditorDraw()
	{
#ifdef WITH_EDITOR
		m_editorIcon.SetPosition(GetTransform()->GetModelMatrix().GetTranslation());
		m_editorIcon.Render(GetGameObject()->GetSceneGraphID());
#endif
	}

	void Component::Light::ShowInInspector()
	{
		p_dirty |= ImGui::ColorEdit3("Ambient", p_ambient.value.Data());
		p_dirty |= ImGui::ColorEdit3("Diffuse", p_diffuse.value.Data());
		p_dirty |= ImGui::ColorEdit3("Specular", p_specular.value.Data());
	}

	void Component::Light::Serialize(CppSer::Serializer& serializer)
	{
		serializer << CppSer::Pair::Key << "Ambient" << CppSer::Pair::Value << p_ambient.value;
		serializer << CppSer::Pair::Key << "Diffuse" << CppSer::Pair::Value << p_diffuse.value;
		serializer << CppSer::Pair::Key << "Specular" << CppSer::Pair::Value << p_specular.value;
	}

	void Component::Light::Deserialize(CppSer::Parser& parser)
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

		shader->SendVec3f(p_ambient.string.c_str(), p_ambient.value);
		shader->SendVec3f(p_diffuse.string.c_str(), p_diffuse.value);
		shader->SendVec3f(p_specular.string.c_str(), p_specular.value);
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
