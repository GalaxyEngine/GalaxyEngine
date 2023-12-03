#include "pch.h"
#include "Core/Input.h"

#include <ranges>
#include <GLFW/glfw3.h>

#include <Wrapper/Window.h>
#include <Core/Application.h>

namespace GALAXY 
{

	void Input::key_callback(GLFWwindow* window, const int key, int scancode, const int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			m_keyPressed[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			m_keyPressed.erase(key);
			m_keyDown.erase(key);
			m_keyRelease[key] = true;
		}
	}

	void Input::mouse_button_callback(GLFWwindow* window, const int button, const int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			m_keyPressed[button] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			m_keyPressed.erase(button);
			m_keyDown.erase(button);
			m_keyRelease[button] = true;
		}
	}

	void Input::scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset)
	{
		m_scrollValue = static_cast<float>(yoffset);
	}

	GLFWwindow* Input::m_window = nullptr;

	std::unordered_map<int, bool> Input::m_keyPressed;

	std::unordered_map<int, bool> Input::m_keyDown;

	std::unordered_map<int, bool> Input::m_keyRelease;

	float Input::m_scrollValue = 0.f;

	Vec2f Input::m_mouseDragDelta = Vec2f(0.f);

	void Input::Initialize()
	{
		m_window = static_cast<GLFWwindow*>(Core::Application::GetInstance().GetWindow()->GetWindow());
		glfwSetKeyCallback(m_window, key_callback);
		glfwSetMouseButtonCallback(m_window, mouse_button_callback);
		glfwSetScrollCallback(m_window, scroll_callback);
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
		double mouseX, mouseY;
		glfwGetCursorPos(m_window, &mouseX, &mouseY);
		mousePosition.x = static_cast<float>(mouseX);
		mousePosition.y = static_cast<float>(mouseY);
	}

	Vec2f Input::GetMousePosition()
	{
		Vec2f mousePosition;
		double mouseX, mouseY;
		glfwGetCursorPos(m_window, &mouseX, &mouseY);
		mousePosition.x = static_cast<float>(mouseX);
		mousePosition.y = static_cast<float>(mouseY);
		return mousePosition;
	}
}
