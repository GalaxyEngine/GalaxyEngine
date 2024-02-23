#include "pch.h"
#include "Component/CameraComponent.h"

#include "Resource/Scene.h"

#include "Editor/UI/EditorUIManager.h"
#include "Resource/ResourceManager.h"

namespace GALAXY 
{

	void Component::CameraComponent::OnCreate()
	{
		Weak<BaseComponent> self = GetGameObject()->GetComponentWithIndex(p_id);
		GetGameObject()->GetScene()->AddCamera(std::dynamic_pointer_cast<CameraComponent>(self.lock()));
		m_editorIcon.SetIconTexture(Resource::ResourceManager::GetOrLoad<Resource::Texture>(CAMERA_ICON_PATH));
	}

	void Component::CameraComponent::OnDraw()
	{

	}

	void Component::CameraComponent::OnEditorDraw()
	{
		m_editorIcon.SetPosition(GetTransform()->GetModelMatrix().GetTranslation());
		m_editorIcon.Render(GetGameObject()->GetSceneGraphID());

		if (GetGameObject()->IsSelected())
		{
			static auto instance = Wrapper::Renderer::GetInstance();

			Vec3f topleftnear = UnProject(Vec3f{ 0,0, p_near });
			Vec3f topleftfar = UnProject(Vec3f{ 0,0, p_far });

			Vec2f screenResolution = GetScreenResolution();

			Vec3f toprightnear = UnProject(Vec3f{ screenResolution.x,0, p_near });
			Vec3f toprightfar = UnProject(Vec3f{ screenResolution.x,0, p_far });

			Vec3f bottomleftnear = UnProject(Vec3f{ 0, screenResolution.y, p_near });
			Vec3f bottomleftfar = UnProject(Vec3f{ 0, screenResolution.y, p_far });

			Vec3f bottomrightnear = UnProject(Vec3f{ screenResolution, p_near });
			Vec3f bottomrightfar = UnProject(Vec3f{ screenResolution, p_far });

			auto pos = topleftnear;
			auto direction = bottomleftnear;
			instance->DrawLine(pos, direction);

			pos = bottomleftnear;
			direction = bottomrightnear;
			instance->DrawLine(pos, direction);

			pos = bottomrightnear;
			direction = toprightnear;
			instance->DrawLine(pos, direction);

			pos = toprightnear;
			direction = topleftnear;
			instance->DrawLine(pos, direction);

			pos = topleftnear;
			direction = topleftfar;
			instance->DrawLine(pos, direction);

			pos = bottomleftnear;
			direction = bottomleftfar;
			instance->DrawLine(pos, direction);

			pos = bottomrightnear;
			direction = bottomrightfar;
			instance->DrawLine(pos, direction);

			pos = toprightnear;
			direction = toprightfar;
			instance->DrawLine(pos, direction);

			pos = topleftfar;
			direction = bottomleftfar;
			instance->DrawLine(pos, direction);

			pos = bottomleftfar;
			direction = bottomrightfar;
			instance->DrawLine(pos, direction);

			pos = bottomrightfar;
			direction = toprightfar;
			instance->DrawLine(pos, direction);

			pos = toprightfar;
			direction = topleftfar;
			instance->DrawLine(pos, direction);

		}
	}

	void Component::CameraComponent::ShowInInspector()
	{
		DisplayCameraSettings();
	}

	bool Component::CameraComponent::IsVisible() const
	{
		return Editor::UI::EditorUIManager::GetInstance()->GetGameWindow()->IsVisible();
	}

	Vec2i Component::CameraComponent::GetScreenResolution() const
	{
		return Editor::UI::EditorUIManager::GetInstance()->GetGameWindow()->GetImageSize();
	}

	void Component::CameraComponent::SetMainCamera()
	{
		Weak<BaseComponent> self = GetGameObject()->GetComponentWithIndex(p_id);
		GetGameObject()->GetScene()->SetMainCamera(std::dynamic_pointer_cast<CameraComponent>(self.lock()));
	}

}
