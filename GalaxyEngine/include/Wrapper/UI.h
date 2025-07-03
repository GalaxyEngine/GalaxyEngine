#pragma once
#include <GalaxyAPI.h>
#include <galaxymath/Maths.h>

#include "Component/IComponent.h"

#define FONT_DEFAULT_NAME "default"
#define FONT_DEFAULT_ITALIC_NAME "default_italic"
#define FONT_DEFAULT_BOLD_NAME "default_bold"
#define FONT_DEFAULT_BOLD_ITALIC_NAME "default_bold_italic"
#define FONT_DEFAULT_ITALIC_THIN_NAME "default_thin_italic"
#define FONT_DEFAULT_LIGHT_NAME "default_light"

#define FONT_DEFAULT_PATH ENGINE_RESOURCE_FOLDER_NAME"/fonts/Calibri.ttf"
#define FONT_DEFAULT_ITALIC_PATH ENGINE_RESOURCE_FOLDER_NAME"/fonts/Calibri-Italic.ttf"
#define FONT_DEFAULT_BOLD_PATH ENGINE_RESOURCE_FOLDER_NAME"/fonts/Calibri-Bold.ttf"
#define FONT_DEFAULT_BOLD_ITALIC_PATH ENGINE_RESOURCE_FOLDER_NAME"/fonts/Calibri-BoldItalic.ttf"
#define FONT_DEFAULT_ITALIC_THIN_PATH ENGINE_RESOURCE_FOLDER_NAME"/fonts/Calibri-ThinItalic.ttf"
#define FONT_DEFAULT_LIGHT_PATH ENGINE_RESOURCE_FOLDER_NAME"/fonts/Calibri-Light.ttf"

#define FONT_TITLE_1 "Title1"


struct ImFont;
typedef int ImGuiInputTextFlags;
struct ImTextureRef;

namespace GALAXY
{
    namespace Component
    {
        class BaseComponent;
    }

    namespace Resource
    {
        class Texture;
    }

    namespace Wrapper
    {
        class Window;
        
        struct FontData
        {
            std::string name;
            std::filesystem::path path;
            float size;
            ImFont *handle;
        };
            
        using FontMap = std::unordered_map<std::string, FontData>;
        class GALAXY_API FontManager
        {
        public:
            static void Initialize();
                
            static void Destroy();

            static void AddFonts();

            static void RecreateFonts(float dpiScale);

            static void CreateFontGUI(const std::string& name, const std::filesystem::path& fontPath, float size);

            static ImFont* GetFont(const std::string& name);

            static FontManager* GetInstance() { return s_instance; }

        private:
            static FontManager* s_instance;
            
            std::vector<std::string> m_fontsNotSent;
            FontMap m_fonts;
        };

        namespace UI
        {
            void Initialize(const std::unique_ptr<Wrapper::Window>& window, const char* glsl_version);
            void UnInitalize();

            void SetDefaultFontSize(float pixel_size);
            void SetTheme();

            void NewFrame();
            void EndFrame(const std::unique_ptr<Wrapper::Window>& window);
            
            void TextFont(const std::string& font, const std::string& text);
            void PushFont(const std::string& fontName);
            void PopFont();

            bool GALAXY_API InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0);

            bool GALAXY_API DrawVec3Control(const std::string& label, float* values, float resetValue = 0.0f,
                                            bool lockButton = false, float columnWidth = 100.0f);
            
            bool GALAXY_API TextureButton(uint32_t textureID, const Vec2f& size);
            bool GALAXY_API TextureButton(const Resource::Texture* texture, const Vec2f& size);

            bool GALAXY_API TextureButtonWithText(Resource::Texture* texture, const char* label, const Vec2f& imageSize,
                                                  const Vec2f& uv0 = {0, 0}, const Vec2f& uv1 = {1, 1},
                                                  int frame_padding = 0, const Vec4f& bg_col = Vec4f(0, 0, 0, 1),
                                                  const Vec4f& tint_col = Vec4f(1, 1, 1, 1));
            
            bool GALAXY_API TextureToggleButtonWithText(uint32_t texture, const char* label, bool* toggle,
                                                        const Vec2f& imageSize, const Vec2f& uv0 = {0, 0},
                                                        const Vec2f& uv1 = {1, 1}, int frame_padding = 0,
                                                        const Vec4f& bg_col = Vec4f(0, 0, 0, 1),
                                                        const Vec4f& tint_col = Vec4f(1, 1, 1, 1));

            bool GALAXY_API TextureToggleButtonWithText(Resource::Texture* texture, const char* label, bool* toggle,
                                                        const Vec2f& imageSize, const Vec2f& uv0 = {0, 0},
                                                        const Vec2f& uv1 = {1, 1}, int frame_padding = 0,
                                                        const Vec4f& bg_col = Vec4f(0, 0, 0, 1),
                                                        const Vec4f& tint_col = Vec4f(1, 1, 1, 1));

            void GALAXY_API TextureImage(Resource::Texture* texture, Vec2f size, const Vec2i& uv0 = Vec2i(0, 0),
                const Vec2i& uv1 = Vec2i(1, 1));
            
            void GALAXY_API TextureImage(uint32_t texture, Vec2f size, const Vec2i& uv0 = Vec2i(0, 0),
                                         const Vec2i& uv1 = Vec2i(1, 1));


            bool GALAXY_API IconButton(const char* label, uint32_t icon, const Vec2f& icon_size, float spacing = 4.0f, const Vec2f& button_size = {});

            void SetNearestFiltering();

            void ResetNearestFiltering();

            void GALAXY_API ToggleButton(const char* name, bool* toggle, const Vec2f& size = Vec2f(0, 0));

            void GALAXY_API TextSelectable(const std::string& label, const Vec4f& color = Vec4f(1));

            bool GALAXY_API Splitter(bool split_vertically, float thickness, float* size1, float* size2,
                                     float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

            std::shared_ptr<Component::BaseComponent> ComponentPopup();

            float GALAXY_API DeltaTime();

            void GALAXY_API SetNextItemOpen(bool open = true);
            bool GALAXY_API TreeNode(const char* treeName);
            void GALAXY_API TreePop();

            void GALAXY_API TreePush(const void* ptr_id, float indent);
            void GALAXY_API TreePop(float indent);

            void GALAXY_API PushID(size_t id);
            void GALAXY_API PopID();
            bool GALAXY_API Button(const char* buttonName, const Vec2f& buttonSize = Vec2f(0));
            bool GALAXY_API DragFloat(const char* label, float* value, float speed = 1.0f, float min = 0.0f,
                                      float max = 0.0f, const char* format = "%.3f", int flags = 0);
            bool GALAXY_API DragInt(const char* label, int* value, float speed = 1.0f, int min = 0, int max = 0,
                                    const char* format = "%d", int flags = 0);
            bool GALAXY_API DragDouble(const char* label, double* value, float speed = 1.0f, double min = 0.0f,
                                       double max = 0.0f, const char* format = "%.3f", int flags = 0);
            void GALAXY_API SameLine();

            float GALAXY_API GetScaleFactor();

            void GALAXY_API Test(Component::BaseComponent* component);

            bool GALAXY_API Checkbox(const char* label, bool* checked);

            bool GALAXY_API Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);  

            void Spinner(const char* label, float radius = 7.5f, float thickness = 1.5f, uint32_t color = 0xFFFFFFFF,
                         float speed = 5.f);
            void ArcSpinner(const char* label, float radius = 7.5f, float thickness = 1.5f, uint32_t color = 0xFFFFFFFF,
                            float speed = 5.f);
            void LinesSpinner(const char* label, float = 7.5f, float thickness = 1.5f, uint32_t color = 0xFFFFFFFF,
                              float speed = 5.f);
            void PulsatingDotsSpinner(const char* label, float = 7.5f, float dot_radius = 1.5f,
                                      uint32_t color = 0xFFFFFFFF, float speed = 5.f);
            void ConcentricSpinners(const char* label, float outer_radius = 7.5f, float thickness = 1.5f,
                                    uint32_t color = 0xFFFFFFFF,
                                    float speed = 5.f);
            void LineFadeSpinner(const char* label, float radius = 7.5f, float thickness = 1.5f,
                                 uint32_t color = 0xFFFFFFFF, float speed = 5.f);


            void DisableIniFile(bool value);
        }
    }
}
using namespace Wrapper;
