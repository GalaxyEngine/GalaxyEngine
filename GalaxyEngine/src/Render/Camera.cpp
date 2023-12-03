#include "pch.h"

#include "Render/Camera.h"
#include "Render/EditorCamera.h"
#include "Render/Framebuffer.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"
#include "Core/Input.h"

#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/PostProcessShader.h"

#include "Editor/Gizmo.h"

namespace GALAXY
{

	Render::Camera::Camera()
	{
		m_framebuffer = new Render::Framebuffer(Core::Application::GetInstance().GetWindow()->GetSize());
		m_framebuffer->SetClearColor(p_clearColor);
	}

	Render::Camera::~Camera()
	{
		delete m_framebuffer;
	}

	void Render::Camera::DisplayCameraSettings()
	{
		const float contentWidth = 200.f;

		// Use the content width as the width for the controls
		ImGui::PushItemWidth(contentWidth);
		ImGui::SliderFloat("FOV", &p_fov, 25.f, 120.f);
		ImGui::DragFloatRange2("Near/Far", &p_near, &p_far, 0.1f);
		ImGui::ColorPicker4("Clear Color", &p_clearColor.x);
		std::string buttonName;
		if (const Shared<Resource::PostProcessShader> shader = m_framebuffer->GetPostProcessShader().lock())
			buttonName = shader->GetFileInfo().GetFileNameNoExtension();
		else
			buttonName = "Set Post Process Shader";
		if (ImGui::Button(buttonName.c_str(), Vec2f(contentWidth, 0)))
		{
			ImGui::OpenPopup("PostProcessPopup");
		}
		ImGui::PopItemWidth();
		if (const Shared<Resource::PostProcessShader> ppShader = Resource::ResourceManager::GetInstance()->ResourcePopup<Resource::PostProcessShader>("PostProcessPopup").lock())
		{
			m_framebuffer->SetPostProcessShader(ppShader);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			m_framebuffer->SetPostProcessShader(Weak<Resource::PostProcessShader>());
		}
	}

   Physic::Ray Render::Camera::ScreenPointToRay(const Vec3f &point)
   {
	   const Vec3f position = GetTransform()->GetWorldPosition();
	   const Vec3f unprojectPoint = UnProject({point.x, point.y, p_far});
	   const Vec3f direction = (unprojectPoint - position).GetNormalize();
		return Physic::Ray(position, direction, p_far);
   }

	Vec2i Render::Camera::GetScreenResolution() const
	{
		return Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow()->GetImageSize();
	}

	Weak<Resource::Texture> Render::Camera::GetRenderTexture() const
	{
		return m_framebuffer->GetRenderTexture();
	}

	Shared<Render::EditorCamera> Render::Camera::GetEditorCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetEditorCamera();
	}

	Weak<Render::Camera> Render::Camera::GetCurrentCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetCurrentCamera();
	}

	bool Render::Camera::IsVisible() const
	{
		return Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow()->IsVisible();
	}

	void Render::Camera::Begin() const
	{
		m_framebuffer->Begin();
	}

	void Render::Camera::End() const
	{
		m_framebuffer->End();
	}

}