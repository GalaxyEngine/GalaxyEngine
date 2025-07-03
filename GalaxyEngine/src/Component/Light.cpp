#include "pch.h"
#include "Component/Light.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"

#include "Render/LightManager.h"

#include "Core/GameObject.h"
#include "Render/Camera.h"

#include "Utils/Define.h"


#ifdef WITH_EDITOR
#include "Editor/UI/IconManager.h"
#endif


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

		p_shadowMap.ShowInInspector();
	}

	uint32_t Component::Light::GetIcon()
	{
		return Editor::UI::IconManager::LightComponentIcon;
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
		if (p_shadowMap.IsEnabled())
		{
			shader->SendInt("camera.hasDepthMap", true);
			shader->SendTexture("camera.depthMap", p_shadowMap.GetRenderTexture().get());
		}
		else
		{
			shader->SendInt("camera.hasDepthMap", false);
		}
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
		auto currentCamera = Render::Camera::GetCurrentCamera();
		// if (!currentCamera)
		{
			return Mat4::CreateOrthographicMatrix(-10.f, 10.f, -10.f, 10.f,
												   p_shadowMap.GetNear(), p_shadowMap.GetFar());
		}

		// Retrieve the camera's frustum corners in world space
		std::array<Vec3f, 8> frustumCorners = currentCamera->GetFrustumCorners();

		// Get the light's view matrix (world-to-light-space transform)
		Mat4 lightView = GetViewMatrix();

		// Initialize the bounding box in light space
		Vec3f minPoint(FLT_MAX, FLT_MAX, FLT_MAX);
		Vec3f maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		// Transform each frustum corner to light space and expand the bounding box
		for (const auto& corner : frustumCorners)
		{
			Vec3f lightSpaceCorner = lightView * corner; // Assuming operator* applies the transformation

			minPoint.x = std::min(minPoint.x, lightSpaceCorner.x);
			minPoint.y = std::min(minPoint.y, lightSpaceCorner.y);
			minPoint.z = std::min(minPoint.z, lightSpaceCorner.z);

			maxPoint.x = std::max(maxPoint.x, lightSpaceCorner.x);
			maxPoint.y = std::max(maxPoint.y, lightSpaceCorner.y);
			maxPoint.z = std::max(maxPoint.z, lightSpaceCorner.z);
		}

		// Create an orthographic projection matrix that encloses the light-space bounding box.
		// The order of parameters is: left, right, bottom, top, near, far.
		return Mat4::CreateOrthographicMatrix(minPoint.x, maxPoint.x,
											   minPoint.y, maxPoint.y,
											   minPoint.z, maxPoint.z);
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
