#include "pch.h"
#include "Editor/PackageManager.h"

#include "Utils/OS.h"

namespace GALAXY 
{
    Editor::PackagePlatform Editor::PlatformToPackagePlatform(Utils::OS::Platform platform)
    {
        switch (platform)
        {
        case Utils::OS::Platform::Windows:
            return Editor::PackagePlatform::Windows;
        case Utils::OS::Platform::Linux:
            return Editor::PackagePlatform::Linux;
        case Utils::OS::Platform::MacOS:
            return Editor::PackagePlatform::MacOS;
        default:
            return Editor::PackagePlatform::Undefined;
        }
    }

    const char* Editor::SerializePackagePlatformValue(PackagePlatform platform)
    {
        switch (platform)
        {
        case PackagePlatform::Windows:
            return "Windows";
        case PackagePlatform::Linux:
            return "Linux";
        case PackagePlatform::MacOS:
            return "MacOS";
        default:
            return "Undefined";
        }
    }

    const char* Editor::SerializePackagePlatformEnum()
    {
        return "Windows\0Linux\0MacOS\0";
    }

    Editor::CompilerTool Editor::GetUserCompiler()
    {
#ifdef _MSC_VER
        return CompilerTool::MSVC;
#elif defined(__GNUC__)
        return CompilerTool::GCC;
#elif defined(__MINGW32__)
        return CompilerTool::MINGW;
#else
        return CompilerTool::Undefined;
#endif
    }

    const char* Editor::SerializeCompilerToolValue(CompilerTool tool)
    {
        switch (tool)
        {
        case CompilerTool::MSVC:
            return "MSVC";
        case CompilerTool::GCC:
            return "GCC";
        case CompilerTool::MINGW:
            return "MINGW";
        default:
            return "Undefined";
        }
    }

    const char* Editor::SerializeCompilerToolEnum()
    {
        return "MSVC\0GCC\0MINGW\0";
    }

    Editor::PackageManager::PackageManager()
    {
        
    }

    void Editor::PackageManager::PackageProject(bool forceSetBuildPath)
    {
        if (!m_packagePath.has_value() || forceSetBuildPath)
        {
            auto path = Utils::OS::OpenFolderDialog();

            if (path.empty())
            {
                // No path selected
                return;
            }
            if (!SetPackagePath(path))
            {
                PrintLog("Cancel package project");
                return;
            }
        }

        // TODO: Package the project
    }

    void Editor::PackageManager::DrawSettings()
    {
        if (ImGui::BeginPopupModal("Package Settings"))
        {
            if (ImGui::BeginCombo("Platform", SerializePackagePlatformValue(m_platform)))
            {
                PackagePlatform userPlatform = GetUserPlatform();
                for (int i = 0; i < static_cast<int>(PackagePlatform::Undefined); i++)
                {
                    const bool is_selected = (m_platform == static_cast<PackagePlatform>(i));
                    ImGui::BeginDisabled(userPlatform != static_cast<PackagePlatform>(i));
                    if (ImGui::Selectable(SerializePackagePlatformValue(static_cast<PackagePlatform>(i)), is_selected))
                        m_platform = static_cast<PackagePlatform>(i);
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                    ImGui::EndDisabled();
                }
                ImGui::EndCombo();
                ImGui::SetTooltip("Platform used to package the project (only work with the current platform).");
            }

            if (ImGui::BeginCombo("Compiler", SerializeCompilerToolValue(m_compiler)))
            {
                CompilerTool userCompiler = GetUserCompiler();
                for (int i = 0; i < static_cast<int>(CompilerTool::Undefined); i++)
                {
                    const bool is_selected = (m_compiler == static_cast<CompilerTool>(i));
                    ImGui::BeginDisabled(userCompiler != static_cast<CompilerTool>(i));
                    if (ImGui::Selectable(SerializeCompilerToolValue(static_cast<CompilerTool>(i)), is_selected))
                        m_compiler = static_cast<CompilerTool>(i);
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                    ImGui::EndDisabled();
                }
                ImGui::EndCombo();
                ImGui::SetTooltip("Compiler used to compile the project (only work with tool use to compile the engine).");
            }

            // Bottom buttons
            float localY = ImGui::GetContentRegionMax().y - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.y * 2;
            ImGui::SetCursorPosY(localY);
            ImGui::Separator();
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Set Path and Package Project"))
            {
                PackageProject(true);
            }
            ImGui::SameLine();
            if (ImGui::Button("Package Project"))
            {
                PackageProject();
            }
            ImGui::EndPopup();
        }
    }

    void Editor::PackageManager::OpenSettings()
    {
        ImGui::OpenPopup("Package Settings");
    }

    bool Editor::PackageManager::SetPackagePath(const std::filesystem::path& path)
    {
        if (std::filesystem::exists(path) || std::filesystem::is_directory(path))
        {
            m_packagePath = path;
        }
        else
        {
            PrintError("Invalid path selected : %s", path.c_str());
            return false;
        }
        return true;
    }

    Editor::PackagePlatform Editor::PackageManager::GetUserPlatform()
    {
        return PlatformToPackagePlatform(Utils::OS::GetPlatform());
    }
}
