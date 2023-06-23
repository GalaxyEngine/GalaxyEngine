#include "pch.h"
#include "Render/Camera.h"
#include "Core/Application.h"

Render::Camera::Camera()
{
	m_transform = std::make_unique<Component::Transform>();
}

Render::Camera::~Camera()
{
}
Vec2f prevMousePos;
void Render::Camera::Update()
{
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

		if (Approximately(m_transform->GetLocalPosition().z, 180.f, 0.1f))
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

Mat4 Render::Camera::GetViewMatrix()
{
	Mat4 temp;
	const auto z = -GetTransform()->GetForward();
	const Vec3f x = GetTransform()->GetRight();
	const Vec3f y = GetTransform()->GetUp();
	const Vec3f delta = Vec3f(-x.Dot(this->GetTransform()->GetWorldPosition() - z), -y.Dot(this->GetTransform()->GetWorldPosition() - z), -z.Dot(this->GetTransform()->GetWorldPosition() - z));
	for (int i = 0; i < 3; i++)
	{
		temp[i][0] = x[i];
		temp[i][1] = y[i];
		temp[i][2] = z[i];
		temp[3][i] = delta[i];
	}
	temp[3][3] = 1;
	return temp;
}

Component::Transform* Render::Camera::GetTransform()
{
	return m_transform.get();
}

Mat4 Render::Camera::GetProjectionMatrix()
{
	float s = 1.0f / ((p_aspectRatio)*std::atan((p_fov / 2.0f) * DegToRad));
	float s2 = 1.0f / std::atan((p_fov / 2.0f) * DegToRad);
	float param1 = -(p_far + p_near) / (p_far - p_near);
	float param2 = -(2 * p_near * p_far) / (p_far - p_near);
	Mat4 out;
	out[0][0] = s;
	out[1][1] = s2;
	out[2][2] = param1;
	out[2][3] = -1;
	out[3][2] = param2;
	return out;
}

Mat4 Render::Camera::GetViewProjectionMatrix()
{
	return GetViewMatrix() * GetProjectionMatrix();
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
