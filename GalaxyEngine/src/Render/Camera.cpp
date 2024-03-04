#include "pch.h"

#include "Render/Camera.h"
#include "Render/Framebuffer.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Core/Input.h"

#include "Resource/ResourceManager.h"
#include "Resource/PostProcessShader.h"
#include "Resource/Scene.h"

#include "Physic/2D/Rect.h"
#include "Physic/Ray.h"

#ifdef WITH_EDITOR
#include "Editor/EditorCamera.h"
#include "Editor/UI/EditorUIManager.h"
#include "Editor/Gizmo.h"
#endif

namespace GALAXY
{

	Render::Camera::Camera()
	{
		p_framebuffer = std::make_shared<Framebuffer>(Core::Application::GetInstance().GetWindow()->GetSize());
	}

	void Render::Camera::DisplayCameraSettings()
	{
		constexpr float contentWidth = 200.f;

		// Use the content width as the width for the controls
		ImGui::PushItemWidth(contentWidth);
		ImGui::SliderFloat("FOV", &p_fov, 25.f, 120.f);
		ImGui::DragFloatRange2("Near/Far", &p_near, &p_far, 0.1f);
		ImGui::ColorEdit4("Clear Color", &p_clearColor.x);
		std::string buttonName;
		if (const Shared<Resource::PostProcessShader> shader = p_framebuffer->GetPostProcessShader().lock())
			buttonName = shader->GetFileInfo().GetFileNameNoExtension();
		else
			buttonName = "Set Post Process Shader";
		if (ImGui::Button(buttonName.c_str(), Vec2f(contentWidth, 0)))
		{
			ImGui::OpenPopup("PostProcessPopup");
		}
		ImGui::PopItemWidth();
		Weak<Resource::PostProcessShader> ppShader;
		if (Resource::ResourceManager::GetInstance()->ResourcePopup("PostProcessPopup", ppShader))
		{
			p_framebuffer->SetPostProcessShader(ppShader);
		}
	}

	Physic::Ray Render::Camera::ScreenPointToRay(const Vec3f& point)
	{
		const Vec3f position = GetTransform()->GetWorldPosition();
		const Vec3f unprojectPoint = UnProject({ point.x, point.y, p_far });
		const Vec3f direction = (unprojectPoint - position).GetNormalize();
		return Physic::Ray(position, direction, p_far);
	}

	Vec2i Render::Camera::GetScreenResolution() const
	{
#ifdef WITH_EDITOR
		return Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow()->GetImageSize();
#else
		return Core::Application::GetInstance().GetWindow()->GetSize();
#endif
	}

	Weak<Resource::Texture> Render::Camera::GetRenderTexture() const
	{
		return p_framebuffer->GetRenderTexture();
	}

#ifdef WITH_EDITOR
	Shared<Render::EditorCamera> Render::Camera::GetEditorCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetEditorCamera();
	}
#endif

	Weak<Render::Camera> Render::Camera::GetCurrentCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetCurrentCamera();
	}

	bool Render::Camera::IsVisible() const
	{
#ifdef WITH_EDITOR
		return Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow()->IsVisible();
#else
		return true;
#endif
	}

	void Render::Camera::Begin() const
	{
		p_framebuffer->Begin(p_framebufferSize);
	}

	void Render::Camera::End() const
	{
		p_framebuffer->End(p_framebufferSize, p_clearColor);
	}

}