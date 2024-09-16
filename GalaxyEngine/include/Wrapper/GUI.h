#pragma once
#include <GalaxyAPI.h>
#include <galaxymath/Maths.h>

#include "Component/IComponent.h"

typedef int ImGuiInputTextFlags;
typedef void* ImTextureID;
namespace GALAXY {
	namespace Component { class BaseComponent; }
	namespace Resource { class Texture; }
	namespace Wrapper
	{
		class Window;
		namespace GUI
		{
			void Initialize(const std::unique_ptr<Wrapper::Window>& window, const char* glsl_version);
			void UnInitalize();

			void SetDefaultFontSize(float pixel_size);
			void SetTheme();

			void NewFrame();
			void EndFrame(const std::unique_ptr<Wrapper::Window>& window);

			bool GALAXY_API InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0);

			bool GALAXY_API DrawVec3Control(const std::string& label, float* values, float resetValue = 0.0f, bool lockButton = false, float columnWidth = 100.0f);

			ImTextureID GetTextureID(const Resource::Texture* texture);

			bool GALAXY_API TextureButton(const Resource::Texture* texture, Vec2f size);

			bool GALAXY_API TextureButtonWithText(Resource::Texture* texture, const char* label, const Vec2f& imageSize, const Vec2f& uv0 = {0, 0}, const Vec2f& uv1 = { 1, 1 }, int frame_padding = 0, const Vec4f& bg_col = Vec4f(0, 0, 0, 1), const Vec4f& tint_col = Vec4f(1, 1, 1, 1));

			bool GALAXY_API TextureToggleButtonWithText(Resource::Texture* texture, const char* label, bool* toggle,
			                                 const Vec2f& imageSize, const Vec2f& uv0 = {0, 0},
			                                 const Vec2f& uv1 = {1, 1}, int frame_padding = 0,
			                                 const Vec4f& bg_col = Vec4f(0, 0, 0, 1),
			                                 const Vec4f& tint_col = Vec4f(1, 1, 1, 1));
			
			void GALAXY_API TextureImage(Resource::Texture* texture, Vec2f size, const Vec2i& uv0 = Vec2i(0, 0), const Vec2i& uv1 = Vec2i(1, 1));

			void SetNearestFiltering();

			void ResetNearestFiltering();

			void GALAXY_API ToggleButton(const char* name, bool* toggle, const Vec2f& size = Vec2f(0, 0));

			void GALAXY_API TextSelectable(const std::string& label, const Vec4f& color = Vec4f(1));

			bool GALAXY_API Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

			std::shared_ptr<Component::BaseComponent> ComponentPopup();

			float GALAXY_API DeltaTime();

			void GALAXY_API SetNextItemOpen(bool open = true);
			bool GALAXY_API TreeNode(const char* treeName);
			void GALAXY_API TreePop();

			void GALAXY_API TreePush(const void* ptr_id, float indent);
			void GALAXY_API TreePop(float indent);
			
			void GALAXY_API PushID(size_t id);
			void GALAXY_API PopID();
			bool GALAXY_API Button(const char* buttonName,const Vec2f& buttonSize = Vec2f(0));
			bool GALAXY_API DragFloat(const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const char* format = "%.3f", int flags = 0);
			bool GALAXY_API DragInt(const char* label, int* value, float speed = 1.0f, int min = 0, int max = 0, const char* format = "%d", int flags = 0);
			bool GALAXY_API DragDouble(const char* label, double* value, float speed = 1.0f, double min = 0.0f, double max = 0.0f, const char* format = "%.3f", int flags = 0);
			void GALAXY_API SameLine();

			float GALAXY_API GetScaleFactor();

			void GALAXY_API Test(Component::BaseComponent* component);
		}
	}
}