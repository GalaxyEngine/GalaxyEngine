#include "pch.h"

#include "Render/Camera.h"
#include "Render/Framebuffer.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Core/Scene.h"

#include "EditorUI/EditorUIManager.h"

namespace GALAXY {

	Render::Camera::Camera()
	{
		m_transform = std::make_unique<Component::Transform>();
		m_framebuffer = new Render::Framebuffer(Core::Application::GetInstance().GetWindow()->GetSize());
	}

	Render::Camera::~Camera()
	{
	}
	Vec2f prevMousePos;
	void Render::Camera::Update()
	{
		if (!EditorUI::EditorUIManager::GetInstance()->GetSceneWindow()->IsHovered())
			return;
		/*TODO:
		 * change input with input class
		 * change delta time with Time class
		*/
		bool fastMode = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
		bool movementSpeed = fastMode ? m_fastMovementSpeed : m_movementSpeed;

		if (ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_LeftArrow))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetRight() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (ImGui::IsKeyDown(ImGuiKey_D) || ImGui::IsKeyDown(ImGuiKey_RightArrow))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetRight() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_UpArrow))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetForward() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (ImGui::IsKeyDown(ImGuiKey_S) || ImGui::IsKeyDown(ImGuiKey_DownArrow))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetForward() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (ImGui::IsKeyDown(ImGuiKey_Q))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetUp() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (ImGui::IsKeyDown(ImGuiKey_E))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetUp() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (ImGui::IsKeyDown(ImGuiKey_R) || ImGui::IsKeyDown(ImGuiKey_PageUp))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (Vec3f::Up() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (ImGui::IsKeyDown(ImGuiKey_F) || ImGui::IsKeyDown(ImGuiKey_PageDown))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-Vec3f::Up() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (m_looking)
		{
			float mouseX = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.01f).x * m_freeLookSensitivity * 0.1f;
			float mouseY = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.01f).y * m_freeLookSensitivity * 0.1f;

			Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
			window->SetMousePosition(prevMousePos);

			if (Approximately(m_transform->GetLocalEulerRotation().z, 180.f, 0.1f))
			{
				mouseX *= -1;
			}

			m_transform->Rotate(Vec3f::Up(), mouseX, Space::World);
			m_transform->Rotate(Vec3f::Right(), mouseY, Space::Local);
		}

		float axis = ImGui::GetIO().MouseWheel;
		if (axis != 0)
		{
			float zoomSensitivity = fastMode ? m_fastZoomSensitivity : m_zoomSensitivity;
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + m_transform->GetForward() * axis * zoomSensitivity);
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			StartLooking();
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			StopLooking();
		}
	}

	void Render::Camera::DisplayCameraSettings()
	{
		float contentWidth = 200.f;

		// Use the content width as the width for the controls
		ImGui::PushItemWidth(contentWidth);
		ImGui::SliderFloat("FOV", &p_fov, 25.f, 120.f);
		ImGui::DragFloatRange2("Near/Far", &p_near, &p_far, 0.1f);
		ImGui::ColorPicker4("Clear Color", &p_clearColor.x);
		ImGui::PopItemWidth();
	}

	Mat4 Render::Camera::GetViewMatrix()
	{
		Mat4 out = Mat4::CreateTransformMatrix(m_transform->GetWorldPosition(), m_transform->GetWorldRotation(), Vec3f(1, 1, -1));
		out = out.CreateInverseMatrix();
		return out;
	}

	Component::Transform* Render::Camera::Transform()
	{
		return m_transform.get();
	}

	Mat4 Render::Camera::GetProjectionMatrix()
	{
		return Mat4::CreateProjectionMatrix(p_fov, p_aspectRatio, p_near, p_far);
	}

	Mat4 Render::Camera::GetViewProjectionMatrix()
	{
		return GetProjectionMatrix() * GetViewMatrix();
	}

	std::weak_ptr<Resource::Texture> Render::Camera::GetRenderTexture()
	{
		return m_framebuffer->GetRenderTexture();
	}

	void Render::Camera::StartLooking()
	{
		m_looking = true;
		Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		prevMousePos = (Vec2f)ImGui::GetMousePos() - window->GetPosition();
		window->SetCursorMode(Wrapper::CursorMode::Hidden);
	}

	void Render::Camera::StopLooking()
	{
		m_looking = false;
		Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		window->SetMousePosition(prevMousePos);
		window->SetCursorMode(Wrapper::CursorMode::Normal);
	}

	std::shared_ptr<Render::Camera> Render::Camera::GetEditorCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetEditorCamera();
	}

	std::weak_ptr<Render::Camera> Render::Camera::GetCurrentCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetCurrentCamera();
	}

	void Render::Camera::SetSize(const Vec2i& framebufferSize)
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
		m_framebuffer->Update(Core::Application::GetInstance().GetWindow()->GetSize());
		Wrapper::Renderer::GetInstance()->BindRenderBuffer(m_framebuffer);
	}

	void Render::Camera::End()
	{
		Wrapper::Renderer::GetInstance()->UnbindRenderBuffer(m_framebuffer);
	}

}