#pragma once
#include "GalaxyAPI.h"
namespace GALAXY::Editor::UI {
	class EditorWindow
	{
	public:
		virtual ~EditorWindow() {}

		inline bool IsOpen() const { return p_open; }
		inline void SetOpen(const bool value) { p_open = value; };

		virtual void Initialize() {}
		virtual void Draw() {}

	protected:
		bool p_open = true;
	};
}
