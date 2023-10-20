#include "pch.h"

#include "Render/Camera.h"
#include "Render/EditorCamera.h"
#include "Render/Framebuffer.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"
#include "Core/Input.h"

#include "EditorUI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/PostProcessShader.h"

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
		float contentWidth = 200.f;

		// Use the content width as the width for the controls
		ImGui::PushItemWidth(contentWidth);
		ImGui::SliderFloat("FOV", &p_fov, 25.f, 120.f);
		ImGui::DragFloatRange2("Near/Far", &p_near, &p_far, 0.1f);
		ImGui::ColorPicker4("Clear Color", &p_clearColor.x);
		std::string buttonName;
		if (auto shader = m_framebuffer->GetPostProcessShader().lock())
			buttonName = shader->GetFileInfo().GetFileNameNoExtension();
		else
			buttonName = "Set Post Process Shader";
		if (ImGui::Button(buttonName.c_str(), Vec2f(contentWidth, 0)))
		{
			ImGui::OpenPopup("PostProcessPopup");
		}
		ImGui::PopItemWidth();
		if (auto ppShader = Resource::ResourceManager::GetInstance()->ResourcePopup<Resource::PostProcessShader>("PostProcessPopup").lock())
		{
			m_framebuffer->SetPostProcessShader(ppShader);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			m_framebuffer->SetPostProcessShader(Weak<Resource::PostProcessShader>());
		}
	}

	Mat4 Render::Camera::GetViewMatrix() const
	{
		Mat4 out = Mat4::CreateTransformMatrix(GetTransform()->GetWorldPosition(), GetTransform()->GetWorldRotation(), Vec3f(1, 1, -1));
		out = out.CreateInverseMatrix();
		return out;
	}

	Mat4 Render::Camera::GetProjectionMatrix() const
	{
		float tanHalfFov = std::tan(p_fov * DegToRad * 0.5f);

		Mat4 projectionMatrix = Mat4();
		projectionMatrix[0][0] = 1.0f / (p_aspectRatio * tanHalfFov);
		projectionMatrix[1][1] = 1.0f / tanHalfFov;
		projectionMatrix[2][2] = (p_far + p_near) / (p_far - p_near);
		projectionMatrix[3][2] = 1.0f;
		projectionMatrix[2][3] = -(2.0f * p_far * p_near) / (p_far - p_near);
		projectionMatrix[3][3] = 0.0f;

		return projectionMatrix;
	}

	Mat4 Render::Camera::GetViewProjectionMatrix() const
	{
		return GetProjectionMatrix() * GetViewMatrix();
	}

	Vec2f Render::Camera::ToViewport(const Vec2f &pos) const
	{
		return {(float)(2.0f * pos.x) / (float)GetScreenResolution().x - 1.0f, (float)1.0f - (2.0f * pos.y) / (float)GetScreenResolution().y};
	}

	Vec2i Render::Camera::GetScreenResolution() const
	{
		return EditorUI::EditorUIManager::GetInstance()->GetSceneWindow()->GetImageSize();
	}

	Vec3f Render::Camera::UnProject(const Vec3f &point) const
	{
		Vec4f mousePosition = Vec4f( ToViewport(point) , 1.f, 1.f );
		Mat4 invVP = GetViewProjectionMatrix().CreateInverseMatrix();
		Vec3f position = GetTransform()->GetWorldPosition();
		Vec4f pos = (invVP * mousePosition);
		return position + (invVP * mousePosition) * point.z;
	}

	std::weak_ptr<Resource::Texture> Render::Camera::GetRenderTexture()
	{
		return m_framebuffer->GetRenderTexture();
	}

	std::shared_ptr<Render::EditorCamera> Render::Camera::GetEditorCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetEditorCamera();
	}

	std::weak_ptr<Render::Camera> Render::Camera::GetCurrentCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetCurrentCamera();
	}

	void Render::Camera::SetSize(const Vec2i &framebufferSize)
	{
		p_framebufferSize = framebufferSize;
		p_aspectRatio = (float)p_framebufferSize.x / (float)p_framebufferSize.y;
	}

	bool Render::Camera::IsVisible()
	{
		return EditorUI::EditorUIManager::GetInstance()->GetSceneWindow()->IsVisible();
	}

	void Render::Camera::Begin()
	{
		m_framebuffer->Begin();
	}

	void Render::Camera::End()
	{
		m_framebuffer->End();
	}

}