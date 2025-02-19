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

        inline const char* SerializeEditorSettingsTabValue(EditorSettingsTab tab)
        {
            switch (tab)
            {
            case EditorSettingsTab::General: return "General";
            case EditorSettingsTab::ExternalTool: return "External Tool";
            case EditorSettingsTab::Appearance: return "Appearance";
            case EditorSettingsTab::Benchmark: return "Benchmark";
            case EditorSettingsTab::EditorInputs: return "Editor Inputs";
            default: return "Invalid";
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

        inline const char* SerializeScriptEditorToolTypeValue(ScriptEditorToolType tool)
        {
            switch (tool)
            {
            case ScriptEditorToolType::None: return "None";
            case ScriptEditorToolType::VisualStudioCode: return "Visual Studio Code";
#ifdef _WIN32
            case ScriptEditorToolType::VisualStudio: return "Visual Studio";
            case ScriptEditorToolType::Rider: return "Rider";
#endif
            case ScriptEditorToolType::Custom: return "Custom";
            default: return "Invalid";
            }
        }

        struct ScriptEditorTool
        {
            ScriptEditorTool(ScriptEditorToolType _type = ScriptEditorToolType::None)
                : type(_type), name(SerializeScriptEditorToolTypeValue(_type)) {}
            
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

            EditorInput(const std::string& name, const Key key) : name(name), key(key)
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

            void AddListElement(EditorSettingsTab tab);

            [[nodiscard]] ScriptEditorToolType GetScriptEditorToolType() const { return m_currentScriptEditorToolType; }
            void SetScriptEditorToolType(const ScriptEditorToolType val);

            void SaveSettings() const;
            void LoadSettings();
            void LoadThumbnail();

            // Save the engine location for xmake to work, by parsing it on load
            static void SaveEngineLocation();

            void InitializeScriptEditorTools();

            [[nodiscard]] Path GetCurrentScriptEditorToolPath() const;

            Path GetDefaultProjectPath() const { return m_defaultProjectPath; }

            bool GetShouldUseVSync() const { return m_useVSync; }
            [[nodiscard]] EditorInputsManager& GetEditorInputsManager() { return m_editorInputsManager; }

            PackageManager& GetPackageManager() { return m_packageManager; }

            bool FocusGameWindowOnPlay() const { return m_focusGameWindowOnPlay; }

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
