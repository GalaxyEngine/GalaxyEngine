#pragma once
#include "GalaxyAPI.h"
namespace GALAXY::EditorUI {
	class EditorWindow
	{
	public:
		~EditorWindow() {}

		inline bool IsOpen() const { return p_open; }
		inline void SetOpen(bool value) { p_open = value; };

		virtual void Initialize() {}
		virtual void Draw() {}

	protected:
		bool p_open = true;
	};
}
