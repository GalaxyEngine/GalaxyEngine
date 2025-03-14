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
		p_editorIcon.SetIconTexture(Resource::ResourceManager::GetOrLoad<Resource::Texture>(LIGHT_ICON_PATH));
#endif
	}

	void Component::Light::OnDestroy()
	{
		const Weak<Light> weak_this = GetGameObject()->GetWeakOfComponent(this);
		Render::LightManager::RemoveLight(weak_this);
	}

#ifdef WITH_EDITOR
	void Component::Light::OnEditorDraw()
	{
		p_editorIcon.SetPosition(GetTransform()->GetModelMatrix().GetTranslation());
		p_editorIcon.Render(GetGameObject()->GetSceneGraphID());
	}

	void Component::Light::ShowInInspector()
	{
		p_dirty |= ImGui::ColorEdit3("Ambient", p_ambient.value.Data());
		p_dirty |= ImGui::ColorEdit3("Diffuse", p_diffuse.value.Data());
		p_dirty |= ImGui::ColorEdit3("Specular", p_specular.value.Data());
	}
#endif

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
		p_dirty = true; // Force dirty (Reason: when creating thumbnail, the values are updated,
		// but no recompute after, i need to do a LightManagerHolder, that will set dirty every other lights
		
		// Always send boolean "is enable"
		shader->SendInt(p_enableString.c_str(), IsEnable());

		// if (!p_dirty)
			// return;

		shader->SendVec3f(p_ambient.string.c_str(), p_ambient.value);
		shader->SendVec3f(p_diffuse.string.c_str(), p_diffuse.value);
		shader->SendVec3f(p_specular.string.c_str(), p_specular.value);
		shader->SendTexture("camera.depthMap", p_shadowMap.GetRenderTexture().get());
	}

	void Component::Light::ResetLightValues(Resource::Shader* shader)
	{
		shader->SendInt(p_enableString.c_str(), false);
	}

	void Component::Light::ComputeLocationName()
	{
		SetDirty();
	}

	Mat4 Component::Light::GetProjectionMatrix() const
	{
        float near_plane = 0.03f, far_plane = 1000.0f;
		return Mat4::CreateOrthographicMatrix(-10.f, 10.f, -10.f, 10.f, near_plane, far_plane);


        Vec2i size = p_shadowMap.GetRenderTexture()->GetSize();
		float p_far = 1000.f;
		float p_near = 0.03f;
		const float width = static_cast<float>(size.x) / 2.f;
		const float height = static_cast<float>(size.y) / 2.f;

		Mat4 orthographicMatrix = Mat4();
		orthographicMatrix[0][0] = 2.0f / (width - -width);
		orthographicMatrix[1][1] = 2.0f / (height - -height);
		orthographicMatrix[2][2] = -2.0f / (p_far - p_near);
		orthographicMatrix[3][0] = -(width + -width) / (width - -width);
		orthographicMatrix[3][1] = -(height + -height) / (height - -height);
		orthographicMatrix[3][2] = -(p_far + p_near) / (p_far - p_near);
		orthographicMatrix[3][3] = 1.0f;
		return orthographicMatrix;
		
	}

	Mat4 lookAt(Vec3f  const & eye, Vec3f  const & center, Vec3f  const & up)
	{
		Vec3f  f = -Vec3f::Normalize(center - eye);
		Vec3f  u = Vec3f::Normalize(up);
		Vec3f  s = Vec3f::Normalize(Vec3f::Cross(f, u));
		u = Vec3f::Cross(s, f);

		Mat4 Result(1);
		Result[0][0] = s.x;
		Result[1][0] = s.y;
		Result[2][0] = s.z;
		Result[0][1] = u.x;
		Result[1][1] = u.y;
		Result[2][1] = u.z;
		Result[0][2] =-f.x;
		Result[1][2] =-f.y;
		Result[2][2] =-f.z;
		Result[3][0] =-Vec3f::Dot(s, eye);
		Result[3][1] =-Vec3f::Dot(u, eye);
		Result[3][2] = Vec3f::Dot(f, eye);
		return Result;
	}

	Mat4 Component::Light::GetViewMatrix() const
	{
		// return lookAt(GetTransform()->GetWorldPosition(), GetTransform()->GetForward(), GetTransform()->GetUp());
		return GetTransform()->GetModelMatrix().GetInverseMatrix();
	}

	Mat4 Component::Light::GetViewProjectionMatrix() const
	{
		return GetProjectionMatrix() * GetViewMatrix();
	}
}
