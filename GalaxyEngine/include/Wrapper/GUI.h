#pragma once
#include <GalaxyAPI.h>
namespace GALAXY {
	namespace Component { class BaseComponent; }
	namespace Resource { class Texture; }
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

			bool TextureButton(Resource::Texture* texture, Vec2f size);

			bool TextureButtonWithText(Resource::Texture* texture, const char* label, const Vec2f& imageSize, const Vec2f& uv0 = {0, 0}, const Vec2f& uv1 = { 1, 1 }, int frame_padding = 0, const Vec4f& bg_col = Vec4f(0, 0, 0, 1), const Vec4f& tint_col = Vec4f(1, 1, 1, 1));

			void TextureToggleButtonWithText(Resource::Texture* texture, const char* label, bool* toggle, const Vec2f& imageSize, const Vec2f& uv0 = { 0, 0 }, const Vec2f& uv1 = { 1, 1 }, int frame_padding = 0, const Vec4f& bg_col = Vec4f(0, 0, 0, 1), const Vec4f& tint_col = Vec4f(1, 1, 1, 1));
			
			void TextureImage(Resource::Texture* texture, Vec2f size, const Vec2i& uv0 = Vec2i(0, 0), const Vec2i& uv1 = Vec2i(1, 1));

			void ToggleButton(const char* name, bool* toggle, const Vec2f& size = Vec2f(0, 0));

			void TextSelectable(const std::string& label, const Vec4f& color = Vec4f(1));

			bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

			std::shared_ptr<Component::BaseComponent> ComponentPopup();

			float DeltaTime();

		}
	}
}