#pragma once
#include <GalaxyAPI.h>

namespace GALX::Wrapper
{
	class Window;
	namespace GUI 
	{
		void Initalize(const std::unique_ptr<Wrapper::Window>& window, const char* glsl_version);
		void UnInitalize();
		void NewFrame();
		void EndFrame(const std::unique_ptr<Wrapper::Window>& window);

		float DeltaTime();

	}
}