#include "pch.h"

#include "Render/EditorCamera.h"
#include "Render/Framebuffer.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"
#include "Core/Input.h"

#include "EditorUI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/PostProcessShader.h"

#define OUTLINE_PATH ENGINE_RESOURCE_FOLDER_NAME"\\shaders\\PostProcess\\Outline\\outline.ppshader"

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

	Component::Transform* Render::EditorCamera::GetTransform()
	{
		return m_transform.get();
	}

	Vec2f prevMousePos;
	void Render::EditorCamera::Update()
	{
		if (!EditorUI::EditorUIManager::GetInstance()->GetSceneWindow()->IsHovered())
			return;
		/*TODO:
		 * change input with input class
		 * change delta time with Time class
		*/
		bool fastMode = Input::IsKeyDown(Key::LEFT_SHIFT) || Input::IsKeyDown(Key::RIGHT_SHIFT);
		float movementSpeed = fastMode ? m_fastMovementSpeed : m_movementSpeed;

		if (Input::IsKeyDown(Key::A) || Input::IsKeyDown(Key::LEFT))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetRight() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (Input::IsKeyDown(Key::D) || Input::IsKeyDown(Key::RIGHT))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetRight() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (Input::IsKeyDown(Key::W) || Input::IsKeyDown(Key::UP))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetForward() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (Input::IsKeyDown(Key::S) || Input::IsKeyDown(Key::DOWN))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetForward() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (Input::IsKeyDown(Key::Q))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (m_transform->GetUp() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (Input::IsKeyDown(Key::E))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-m_transform->GetUp() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (Input::IsKeyDown(Key::R) || Input::IsKeyDown(Key::PAGE_UP))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (Vec3f::Up() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (Input::IsKeyDown(Key::F) || Input::IsKeyDown(Key::PAGE_DOWN))
		{
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + (-Vec3f::Up() * movementSpeed * Wrapper::GUI::DeltaTime()));
		}

		if (m_looking)
		{
			auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.01f);
			float mouseX = delta.x * m_freeLookSensitivity * Wrapper::GUI::DeltaTime();
			float mouseY = delta.y * m_freeLookSensitivity * Wrapper::GUI::DeltaTime();

			Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
			window->SetMousePosition(prevMousePos);

			if (Approximately(m_transform->GetLocalEulerRotation().z, 180.f, 0.1f))
			{
				mouseX *= -1;
			}

			m_transform->Rotate(Vec3f::Up(), -mouseX, Space::World);
			m_transform->Rotate(Vec3f::Right(), -mouseY, Space::Local);
		}

		float axis = Input::GetScrollWheelValue();
		if (axis != 0)
		{
			float zoomSensitivity = fastMode ? m_fastZoomSensitivity : m_zoomSensitivity;
			m_transform->SetLocalPosition(m_transform->GetLocalPosition() + m_transform->GetForward() * axis * zoomSensitivity);
		}

		if (Input::IsMouseButtonPressed(MouseButton::BUTTON_2))
		{
			StartLooking();
		}
		else if (Input::IsMouseButtonReleased(MouseButton::BUTTON_2))
		{
			StopLooking();
		}
	}

	void Render::EditorCamera::StartLooking()
	{
		m_looking = true;
		Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		prevMousePos = (Vec2f)ImGui::GetMousePos() - window->GetPosition();
		window->SetCursorMode(Wrapper::CursorMode::Hidden);
	}

	void Render::EditorCamera::StopLooking()
	{
		m_looking = false;
		Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		window->SetMousePosition(prevMousePos);
		window->SetCursorMode(Wrapper::CursorMode::Normal);
	}
}
