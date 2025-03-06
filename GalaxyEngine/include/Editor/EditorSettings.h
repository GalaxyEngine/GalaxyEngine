#pragma once
#include "GalaxyAPI.h"
#include <optional>
#include <map>
#include <string>

#include "PackageManager.h"
#include "Core/Input.h"
#include "Utils/FileInfo.h"


namespace GALAXY
{
    namespace Core
    {
        class Application;
    }

    namespace Resource
    {
        class Texture;
    }

    namespace Editor
    {
        enum class ApplicationMode
        {
            Play = 0,
            Pause,
            Editor,
        };

        enum class EditorSettingsTab
        {
            General = 0,
            ExternalTool,
            Appearance,
            Benchmark,
            EditorInputs,
        };

        inline const char* to_string(EditorSettingsTab e)
        {
            switch (e)
            {
            case EditorSettingsTab::General: return "General";
            case EditorSettingsTab::ExternalTool: return "ExternalTool";
            case EditorSettingsTab::Appearance: return "Appearance";
            case EditorSettingsTab::Benchmark: return "Benchmark";
            case EditorSettingsTab::EditorInputs: return "EditorInputs";
            default: return "unknown";
            }
        }
        
        enum class ScriptEditorToolType
        {
            None = 0,
            VisualStudioCode = 1,
#ifdef _WIN32
            VisualStudio = 2,
            Rider = 3,
#endif
            Custom = 4,
            Count
        };

        inline const char* to_string(ScriptEditorToolType e)
        {
            switch (e)
            {
            case ScriptEditorToolType::None: return "None";
            case ScriptEditorToolType::VisualStudioCode: return "VisualStudioCode";
#ifdef _WIN32
            case ScriptEditorToolType::VisualStudio: return "VisualStudio";
            case ScriptEditorToolType::Rider: return "Rider";
#endif
            case ScriptEditorToolType::Custom: return "Custom";
            case ScriptEditorToolType::Count: return "Count";
            default: return "unknown";
            }
        }

        struct ScriptEditorTool
        {
            ScriptEditorTool(ScriptEditorToolType _type = ScriptEditorToolType::None)
                : type(_type), name(to_string(_type)) {}
            
            ScriptEditorToolType type;
            std::string name;
            std::filesystem::path path;
        };

        enum class InputAction
        {
            None = 0,
            Forward,
            Backward,
            Left,
            Right,
            Up,
            Down,
            FastMode,
        };

        struct EditorInput
        {
            std::string name;
            Key key;

            EditorInput(std::string name, const Key key) : name(std::move(name)), key(key)
            {
            }

            EditorInput() = default;
        };

        class EditorInputsManager
        {
        public:
            Key GetInputForAction(const InputAction action) const
            {
                return EditorInputs.at(action).key;
            }

            std::unordered_map<InputAction, EditorInput> EditorInputs;

            void Initialize();
        };


        class EditorSettings
        {
        public:
            EditorSettings();
            ~EditorSettings();

            static EditorSettings& GetInstance();

            void Display();

            void TakeScreenShot();

            void DrawTabElement(EditorSettingsTab tab);

            [[nodiscard]] ScriptEditorToolType GetScriptEditorToolType() const;
            void SetScriptEditorToolType(const ScriptEditorToolType val);

            void SaveSettings() const;
            void LoadSettings();
            void LoadThumbnail();

            // Save the engine location for xmake to work, by parsing it on load
            static void SaveEngineLocation();

            void InitializeScriptEditorTools();

            [[nodiscard]] Path GetCurrentScriptEditorToolPath() const;

            Path GetDefaultProjectPath() const;

            bool GetShouldUseVSync() const;
            [[nodiscard]] EditorInputsManager& GetEditorInputsManager();

            PackageManager& GetPackageManager();

            bool FocusGameWindowOnPlay() const;

        private:
            void DisplayTab(EditorSettingsTab tab);

            void DisplayGeneralTab();

            void DisplayExternalToolTab();
            void ChangeOtherScriptTool();

            void DisplayAppearanceTab();

            void DisplayBenchmarkTab();

            void DisplayEditorInputsTab();

            void UpdateScreenShot();

        private:
            friend Core::Application;
            bool m_firstUpdate = false;
            bool m_shouldTakeScreenshot = false;
            bool m_useVSync = true;
            bool m_focusGameWindowOnPlay = true;
            Path m_defaultProjectPath;

            EditorSettingsTab m_selectedTab = EditorSettingsTab::General;

#pragma region External Tools
            std::map<ScriptEditorToolType, ScriptEditorTool> m_scriptEditorTools = {};
#if defined(_WIN32)
            ScriptEditorToolType m_currentScriptEditorToolType = ScriptEditorToolType::VisualStudio;
#else
			ScriptEditorToolType m_currentScriptEditorToolType = ScriptEditorToolType::VisualStudioCode;
#endif
#pragma endregion

            EditorInputsManager m_editorInputsManager;

            Weak<Resource::Texture> m_projectThumbnail = {};

            PackageManager m_packageManager;
        };
    }
}
