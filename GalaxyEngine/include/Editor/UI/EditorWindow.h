#pragma once
#include "GalaxyAPI.h"
namespace GALAXY::Editor::UI {
	class GALAXY_API EditorWindow
	{
	public:
		virtual ~EditorWindow() {}

		inline bool IsOpen() const { return p_open; }
		inline void SetOpen(const bool value) { p_open = value; };

		inline void SetShouldFocus(bool value) { p_shouldFocus = value; }

		virtual void Initialize() {}
		virtual void Draw();

		virtual bool Begin(const char* name);

		inline bool IsVisible() const { return p_visible; }
		inline bool IsHovered() const { return p_isHovered; }
		inline bool IsFocused() const { return p_isFocused; }
	protected:
		bool p_open = true;

		bool p_visible = true;
		bool p_isHovered = false;
		bool p_isFocused = false;

		bool p_shouldFocus = false;
	};
}
