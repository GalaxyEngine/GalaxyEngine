#include "pch.h"

#include "Render/EditorCamera.h"
#include "Render/Framebuffer.h"

#include "Utils/Define.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Core/Input.h"

#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/PostProcessShader.h"
#include "Resource/Scene.h"
#include "Utils/Time.h"

namespace GALAXY
{
	Render::EditorCamera::EditorCamera()
	{
		m_transform = std::make_unique<Component::Transform>();

		m_outlineFramebuffer = new Render::Framebuffer(Core::Application::GetInstance().GetWindow()->GetSize());
		m_outlineFramebuffer->SetPostProcessShader(Resource::ResourceManager::GetOrLoad<Resource::PostProcessShader>(OUTLINE_PATH));
	}

	Render::EditorCamera::~EditorCamera()
	{
		delete m_outlineFramebuffer;
	}

	Vec2f prevMousePos;
	void Render::EditorCamera::Update()
	{
		if (IsLooking() && Input::IsMouseButtonReleased(MouseButton::BUTTON_2))
		{
			StopLooking();
		}

		if (!Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow()->IsHovered())
			return;

		if (Input::IsMouseButtonPressed(MouseButton::BUTTON_2))
		{
			StartLooking();
		}

		if (!m_looking)
			return;

		const bool fastMode = Input::IsKeyDown(Key::LEFT_SHIFT) || Input::IsKeyDown(Key::RIGHT_SHIFT);
		const float movementSpeed = fastMode ? m_fastMovementSpeed : m_movementSpeed;

		const float deltaTime = Utils::Time::DeltaTime();

		if (Input::IsKeyDown(Key::A) || Input::IsKeyDown(Key::LEFT))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetRight() * movementSpeed * deltaTime));
		}

		if (Input::IsKeyDown(Key::D) || Input::IsKeyDown(Key::RIGHT))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetRight() * movementSpeed * deltaTime));
		}

		if (Input::IsKeyDown(Key::W) || Input::IsKeyDown(Key::UP))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetForward() * movementSpeed * deltaTime));
		}

		if (Input::IsKeyDown(Key::S) || Input::IsKeyDown(Key::DOWN))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetForward() * movementSpeed * deltaTime));
		}

		if (Input::IsKeyDown(Key::Q))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetUp() * movementSpeed * deltaTime));
		}

		if (Input::IsKeyDown(Key::E))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetUp() * movementSpeed * deltaTime));
		}

		const Vec2f mousePos = Input::GetMousePosition();
		const Vec2f delta = mousePos - prevMousePos;

		constexpr float maxRotation = 1.f;
		float mouseX = std::clamp(delta.x * m_freeLookSensitivity * deltaTime, -maxRotation, maxRotation);
		const float mouseY = std::clamp(delta.y * m_freeLookSensitivity * deltaTime, -maxRotation, maxRotation);

		const Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		window->SetMousePosition(prevMousePos);

		prevMousePos = Input::GetMousePosition();

		if (GetTransform()->GetUp().y < 0)
		{
			mouseX *= -1;
		}

		m_transform->Rotate(Vec3f::Up(), -mouseX, Space::World);
		m_transform->Rotate(Vec3f::Right(), -mouseY, Space::Local);

		const float axis = Input::GetScrollWheelValue();
		if (axis != 0)
		{
			const float zoomSensitivity = fastMode ? m_fastZoomSensitivity : m_zoomSensitivity;
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + m_transform->GetForward() * axis * zoomSensitivity);
		}
	}

	void Render::EditorCamera::StartLooking()
	{
		m_looking = true;
		const Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		prevMousePos = static_cast<Vec2f>(ImGui::GetMousePos()) - window->GetPosition();
		window->SetCursorMode(Wrapper::CursorMode::Hidden);
	}

	void Render::EditorCamera::StopLooking()
	{
		m_looking = false;
		const Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		window->SetMousePosition(prevMousePos);
		window->SetCursorMode(Wrapper::CursorMode::Normal);
	}
}
