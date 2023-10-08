#pragma once
#include "Core/Input.h"
namespace GALAXY 
{
	bool Input::IsKeyPressed(Key key)
	{
		return m_keyPressed.contains((int)key);
	}

	bool Input::IsKeyReleased(Key key)
	{
		return !m_keyDown.contains((int)key) && !m_keyPressed.contains((int)key);
	}

	bool Input::IsKeyDown(Key key)
	{
		return m_keyDown.contains((int)key);
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		return m_keyPressed.contains((int)button);
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		return m_keyRelease.contains((int)button);
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		return m_keyDown.contains((int)button);
	}

	float Input::GetScrollWheelValue()
	{
		return m_scrollValue;
	}

	Vec2f Input::GetMouseDragDelta()
	{
		return m_mouseDragDelta;
	}
}
