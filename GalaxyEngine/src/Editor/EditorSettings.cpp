#include "pch.h"

#include <numeric>

#include "Editor/EditorSettings.h"

#include "Core/Application.h"
#include "Core/Input.h"

#include "Render/Camera.h"

#include "Editor/EditorCamera.h"

#include "Resource/ResourceManager.h"

#include "Scripting/ScriptEngine.h"

#include "Wrapper/ImageLoader.h"

#include "Utils/FileInfo.h"
#include "Utils/OS.h"

namespace GALAXY
{
    void Editor::EditorInputsManager::Initialize()
    {
        // Default values
        EditorInputs[InputAction::Forward] = EditorInput("Move Forward", Key::W);
        EditorInputs[InputAction::Backward] = EditorInput("Move Backward", Key::S);
        EditorInputs[InputAction::Left] = EditorInput("Move Left", Key::A);
        EditorInputs[InputAction::Right] = EditorInput("Move Right", Key::D);
        EditorInputs[InputAction::Up] = EditorInput("Move Up", Key::E);
        EditorInputs[InputAction::Down] = EditorInput("Move Down", Key::Q);
        EditorInputs[InputAction::FastMode] = EditorInput("Fast Mode", Key::LEFT_SHIFT);
    }

    Editor::EditorSettings::EditorSettings()
    {
    }

    Editor::EditorSettings::~EditorSettings()
    {
    }

    Editor::EditorSettings& Editor::EditorSettings::GetInstance()
    {
        return Core::Application::GetInstance().GetEditorSettings();
    }

    void Editor::EditorSettings::Display()
    {
        if (ImGui::BeginPopupModal("Editor Settings", nullptr/*, ImGuiWindowFlags_NoResize*/))
        {
            constexpr float buttonSizeY = 30;
            static Editor::EditorSettings copySettings = *this;
            static float leftSize = 100.f * Wrapper::GUI::GetScaleFactor();
            static float rightSize;
            static Vec2f previousSize = Vec2f(0);
            const Vec2f newSize = ImGui::GetContentRegionAvail();
            if (m_firstUpdate)
            {
                copySettings = *this;
                m_firstUpdate = false;
            }
            if (newSize != previousSize)
            {
                // When resize reset the size of the right size
                rightSize = ImGui::GetContentRegionAvail().x - leftSize;
                previousSize = newSize;
            }

            Wrapper::GUI::Splitter(true, 2, &leftSize, &rightSize, 10, 10);

            ImGui::BeginChild("List", Vec2f(leftSize, ImGui::GetContentRegionAvail().y - buttonSizeY), false);
            AddListElement(EditorSettingsTab::General);
            AddListElement(EditorSettingsTab::ExternalTool);
            AddListElement(EditorSettingsTab::Appearance);
            AddListElement(EditorSettingsTab::Benchmark);
            AddListElement(EditorSettingsTab::EditorInputs);
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Panel", Vec2f(rightSize, ImGui::GetContentRegionAvail().y - buttonSizeY), true);

            DisplayTab(m_selectedTab);

            ImGui::EndChild();

            ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 100.f * Wrapper::GUI::GetScaleFactor());

            if (ImGui::Button("Cancel"))
            {
                m_firstUpdate = true;
                *this = copySettings;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Save"))
            {
                SaveSettings();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void Editor::EditorSettings::TakeScreenShot()
    {
        m_shouldTakeScreenshot = true;
    }

    void Editor::EditorSettings::AddListElement(const EditorSettingsTab tab)
    {
        if (ImGui::Selectable(SerializeEditorSettingsTabValue(tab)))
        {
            m_selectedTab = tab;
        }
    }

    void Editor::EditorSettings::DisplayTab(const EditorSettingsTab tab)
    {
        switch (tab)
        {
        case EditorSettingsTab::General:
            DisplayGeneralTab();
            break;
        case EditorSettingsTab::ExternalTool:
            DisplayExternalToolTab();
            break;
        case EditorSettingsTab::Appearance:
            DisplayAppearanceTab();
            break;
        case EditorSettingsTab::Benchmark:
            DisplayBenchmarkTab();
            break;
        case EditorSettingsTab::EditorInputs:
            DisplayEditorInputsTab();
            break;
        default:
            break;
        }
    }

	void Editor::EditorSettings::DisplayGeneralTab()
	{
		if (ImGui::Checkbox("Enable VSync", &m_useVSync))
		{
			Core::Application::GetInstance().GetWindow()->SetVSync(m_useVSync);
		}
	}

    void Editor::EditorSettings::ChangeOtherScriptTool()
    {
        auto out = Utils::OS::OpenDialog({{"Script Editor Tool", "exe"}});
        if (!out.empty())
        {
            m_otherScriptEditorToolPath = out;
            m_scriptEditorToolsString[ScriptEditorTool::Custom] = Path(out).stem().string();
        }
    }

    void Editor::EditorSettings::DisplayExternalToolTab()
    {
        ScriptEditorTool externalToolID = Core::Application::GetInstance().GetEditorSettings().GetScriptEditorTool();
        if (ImGui::BeginCombo("Script Editor Tool", m_scriptEditorToolsString[externalToolID].c_str()))
        {
            for (auto& i : m_scriptEditorToolsString)
            {
                bool isOther = i.first == ScriptEditorTool::Custom;
                if (ImGui::Selectable(i.second.c_str(), i.first == externalToolID,
                                      ImGuiSelectableFlags_AllowItemOverlap))
                {
                    SetScriptEditorTool(i.first);

                    if (isOther && !m_otherScriptEditorToolPath.has_value())
                    {
                        ChangeOtherScriptTool();
                    }

                    continue;
                }
                if (!isOther || !m_otherScriptEditorToolPath.has_value())
                    continue;
                ImGui::SameLine();
                if (ImGui::SmallButton("X"))
                {
                    SetScriptEditorTool(ScriptEditorTool::None);
                    m_otherScriptEditorToolPath.reset();
                    i.second = "Other";
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Compile code"))
        {
            Scripting::ScriptEngine::CompileCode();
        }
        if (ImGui::Button("Reload Project DLL"))
        {
            Scripting::ScriptEngine::GetInstance()->ReloadDLL();
        }
        if (ImGui::Button("Generate solution"))
        {
            Scripting::ScriptEngine::GenerateSolution(GetScriptEditorTool());
        }
        if (ImGui::Button("Open solution"))
        {
            Scripting::ScriptEngine::OpenSolution(GetScriptEditorTool());
        }
    }

    void Editor::EditorSettings::DisplayAppearanceTab()
    {
        ImGui::TextUnformatted("Project Thumbnail");
        ImGui::TreePush("Project Thumbnail");
        Wrapper::GUI::TextureImage(m_projectThumbnail.lock().get(), Vec2f(128, 128), {0, 1}, {1, 0});
        if (ImGui::Button("Take Screenshot to set Project Thumbnail"))
            TakeScreenShot();
        ImGui::TreePop();

        ImGui::ShowStyleEditor();
    }

    void Editor::EditorSettings::DisplayBenchmarkTab()
    {
        static float benchmarkTimeSeconds = 60;
        static float benchmarkUpdateFrequency = 0.5f;

        static Benchmark& benchmark = Core::Application::GetInstance().GetBenchmark();

        ImGui::BeginDisabled(benchmark.IsRunning());

        ImGui::InputFloat("Benchmark Time (seconds)", &benchmarkTimeSeconds, 0.1f, 0, "%.1f");
        ImGui::InputFloat("Update Frequency (seconds)", &benchmarkUpdateFrequency, 0.1f, 0, "%.1f");
        if (ImGui::Button("Start Benchmark"))
        {
            auto benchmarkSettings = BenchmarkSettings();
            benchmarkSettings.time = benchmarkTimeSeconds;
            benchmarkSettings.updateFrequency = benchmarkUpdateFrequency;
            benchmark.SetBenchmarkSettings(benchmarkSettings);
            benchmark.StartBenchmark();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        if (benchmark.IsRunning())
        {
            ImGui::Text("Average FPS %f", benchmark.GetAverageFPS());
        }
    }

    void Editor::EditorSettings::DisplayEditorInputsTab()
    {
        static int currentInputChange = -1;
        const float buttonSize = ImGui::GetContentRegionAvail().x * 0.25f;
        for (auto& input : m_editorInputsManager.EditorInputs)
        {
            int index = static_cast<int>(input.first);
            ImGui::PushID(index);
            ImGui::TextUnformatted(input.second.name.c_str());
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - buttonSize);
            std::string keyName = (currentInputChange == index)? "<>": Input::GetKeyName(input.second.key);

            if (ImGui::Button(keyName.c_str(), Vec2f(buttonSize, 0)) && currentInputChange == -1)
            {
                currentInputChange = index;
            }
            if (currentInputChange == index)
            {
                Key keyPressed = Input::GetKeyPressed();
                if (keyPressed != Key::NONE)
                {
                    input.second.key = keyPressed;
                    currentInputChange = -1;
                }
            }
            ImGui::PopID();
        }
    }

    void Editor::EditorSettings::UpdateScreenShot()
    {
        if (!m_shouldTakeScreenshot)
            return;
        m_shouldTakeScreenshot = false;

        auto renderer = Wrapper::Renderer::GetInstance();

        Vec2i frameBufferSize = Core::Application::GetInstance().GetWindow()->GetSize();

        Wrapper::Image imageData;
        imageData.size = frameBufferSize;
        imageData.data = new uint8_t[frameBufferSize.x * frameBufferSize.y * 4];

        renderer->BindRenderBuffer(Render::Camera::GetEditorCamera()->GetFramebuffer().get());
        renderer->ReadPixels(imageData.size, imageData.data);

        std::filesystem::path thumbnailPath = Resource::ResourceManager::GetProjectPath() / PROJECT_THUMBNAIL_PATH;

        PrintLog("Save project thumbnail to %s", thumbnailPath.generic_string().c_str());
        Wrapper::ImageLoader::SaveImage(thumbnailPath.generic_string().c_str(), imageData);
        delete[] imageData.data;

        m_projectThumbnail = Resource::ResourceManager::ReloadResource<Resource::Texture>(thumbnailPath);
    }

	void Editor::EditorSettings::SaveSettings() const
	{
		CppSer::Serializer serializer(Utils::OS::GetUserAppDataFolder() / EDITOR_SETTINGS_PATH);
		serializer << CppSer::Pair::BeginMap << "Editor Settings";
		serializer << CppSer::Pair::Key << "Use VSync" << CppSer::Pair::Value << static_cast<bool>(m_useVSync);
		serializer << CppSer::Pair::Key << "Script Editor Tool" << CppSer::Pair::Value << static_cast<int>(GetScriptEditorTool());
		if (m_otherScriptEditorToolPath.has_value())
			serializer << CppSer::Pair::Key << "Other Script Editor Tool" << CppSer::Pair::Value << m_otherScriptEditorToolPath.value();
        for (auto input : m_editorInputsManager.EditorInputs)
        {
            serializer << CppSer::Pair::Key << "Key " + input.second.name << CppSer::Pair::Value << (int)input.second.key;
        }
        serializer << CppSer::Pair::EndMap << "Editor Settings";
	}

	void Editor::EditorSettings::LoadSettings()
	{
		InitializeScriptEditorTools();
		Path settingsPath = Utils::OS::GetUserAppDataFolder() / EDITOR_SETTINGS_PATH;
		CppSer::Parser parser(settingsPath);
		if (!parser.IsFileOpen())
		{
			PrintError("Can't open %s", settingsPath.string().c_str());
			return;
		}
		m_useVSync = parser["Use VSync"].As<bool>();
		auto scriptEditorTool = static_cast<Editor::ScriptEditorTool>(parser["Script Editor Tool"].As<int>());
		
		if (m_scriptEditorToolsString.contains(scriptEditorTool))
			SetScriptEditorTool(scriptEditorTool);

		auto otherScriptEditorTool = parser["Other Script Editor Tool"].As<std::string>();
		if (!otherScriptEditorTool.empty())
		{
			m_otherScriptEditorToolPath = otherScriptEditorTool;
			m_scriptEditorToolsString[ScriptEditorTool::Custom] = Path(otherScriptEditorTool).stem().string();
		}
        m_editorInputsManager.Initialize();
        for (auto& input : m_editorInputsManager.EditorInputs)
        {
            int key = parser["Key " + input.second.name].As<int>();
            if (key != 0)
                input.second.key = static_cast<Key>(key);
        }
	}

	void Editor::EditorSettings::LoadThumbnail()
	{
		std::filesystem::path thumbnailPath = Resource::ResourceManager::GetProjectPath() / PROJECT_THUMBNAIL_PATH;
		m_projectThumbnail = Resource::ResourceManager::GetOrLoad<Resource::Texture>(thumbnailPath);
	}

    void Editor::EditorSettings::InitializeScriptEditorTools()
    {
        m_scriptEditorToolsString[ScriptEditorTool::None] = "None";
#ifdef _WIN32
        m_scriptEditorToolsString[ScriptEditorTool::VisualStudio] = "Visual Studio";
        if (ShellExecute(NULL, "open", "rider64.exe", NULL, NULL, SW_SHOWNORMAL) <= (HINSTANCE)32)
        {
            m_scriptEditorToolsString[ScriptEditorTool::Rider] = "Rider";
        }
#endif
        m_scriptEditorToolsString[ScriptEditorTool::VisualStudioCode] = "Visual Studio Code";
        m_scriptEditorToolsString[ScriptEditorTool::Custom] = "Custom";
    }
}
