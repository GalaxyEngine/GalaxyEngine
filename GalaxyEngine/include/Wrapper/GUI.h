#pragma once
#include <GalaxyAPI.h>
namespace GALAXY {
	namespace Component { class BaseComponent; }
	namespace Wrapper
	{
		class Window;
		namespace GUI
		{
			void Initalize(const std::unique_ptr<Wrapper::Window>& window, const char* glsl_version);
			void UnInitalize();
			void NewFrame();
			void EndFrame(const std::unique_ptr<Wrapper::Window>& window);

			bool DrawVec3Control(const std::string& label, float* values, float resetValue = 0.0f, bool lockButton = false, float columnWidth = 100.0f);

			std::shared_ptr<Component::BaseComponent> ComponentPopup();

			float DeltaTime();

		}
	}
}