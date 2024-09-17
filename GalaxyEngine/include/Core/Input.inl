#pragma once
#include "Core/Input.h"

namespace GALAXY 
{
	bool Input::IsKeyPressed(Key key)
	{
		return m_keyPressed.contains(static_cast<int>(key));
	}

	bool Input::IsKeyReleased(Key key)
	{
		return !m_keyDown.contains(static_cast<int>(key)) && !m_keyPressed.contains(static_cast<int>(key));
	}

	bool Input::IsKeyDown(Key key)
	{
		return m_keyDown.contains(static_cast<int>(key));
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		return m_keyPressed.contains(static_cast<int>(button));
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		return m_keyRelease.contains(static_cast<int>(button));
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		return m_keyDown.contains(static_cast<int>(button));
	}

	float Input::GetScrollWheelValue()
	{
		return m_scrollValue;
	}

	inline Key Input::GetKeyPressed()
	{
		for (const auto& i : m_keyPressed)
		{
			return static_cast<Key>(i.first);
		}
		return Key::NONE;
	}

	Vec2f Input::GetMouseDragDelta()
	{
		return m_mouseDragDelta;
	}
}
