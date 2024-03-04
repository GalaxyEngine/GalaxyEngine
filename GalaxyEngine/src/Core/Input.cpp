#include "pch.h"
#include "Core/Input.h"

#include <ranges>

#include <Wrapper/Window.h>

#include <Core/Application.h>

namespace GALAXY 
{
	std::unordered_map<int, bool> Input::m_keyPressed;

	std::unordered_map<int, bool> Input::m_keyDown;

	std::unordered_map<int, bool> Input::m_keyRelease;

	float Input::m_scrollValue = 0.f;

	Vec2f Input::m_mouseDragDelta = Vec2f(0.f);

	void Input::key_callback(KeyEvent keyState, int key)
	{
		if (keyState == KeyEvent::Pressed)
		{
			m_keyPressed[key] = true;
		}
		else if (keyState == KeyEvent::Released)
		{
			m_keyPressed.erase(key);
			m_keyDown.erase(key);
			m_keyRelease[key] = true;
		}
	}

	void Input::mouse_button_callback(KeyEvent keyState, int button)
	{
		if (keyState == KeyEvent::Pressed)
		{
			m_keyPressed[button] = true;
		}
		else if (keyState == KeyEvent::Released)
		{
			m_keyPressed.erase(button);
			m_keyDown.erase(button);
			m_keyRelease[button] = true;
		}
	}

	void Input::scroll_callback(const double yoffset)
	{
		m_scrollValue = static_cast<float>(yoffset);
	}

	void Input::Update()
	{
		static Vec2f lastMousePosition = GetMousePosition();
		m_keyRelease.clear();
		for (const int& key : m_keyPressed | std::views::keys)
		{
			m_keyDown[key] = true;
		}
		m_keyPressed.clear();
		m_scrollValue = 0.f;
		const Vec2f mousePosition = GetMousePosition();
		m_mouseDragDelta = lastMousePosition - GetMousePosition();

		lastMousePosition = mousePosition;
	}

	void Input::GetMousePosition(Vec2f& mousePosition)
	{
		mousePosition = Core::Application::GetInstance().GetWindow()->GetMousePosition();
	}

	Vec2f Input::GetMousePosition()
	{
		return Core::Application::GetInstance().GetWindow()->GetMousePosition();;
	}
}
