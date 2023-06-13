#pragma once
#include "GalaxyAPI.h"
namespace GALAXY::EditorUI {
	class EditorWindow
	{
	public:
		~EditorWindow() {}

		bool IsOpen() const { return p_open; }
		void SetOpen(bool value) { p_open = value; };

		virtual void Draw() {}

	protected:
		bool p_open = true;
	};
}
