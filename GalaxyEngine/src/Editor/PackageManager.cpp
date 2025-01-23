#include "pch.h"
#include "Editor/PackageManager.h"

#include "Core/Application.h"
#include "Resource/ResourceManager.h"
#include "Utils/OS.h"

namespace GALAXY 
{
    Editor::PackagePlatform Editor::PlatformToPackagePlatform(Utils::OS::Platform platform)
    {
        switch (platform)
        {
        case Utils::OS::Platform::Windows:
            return PackagePlatform::Windows;
        case Utils::OS::Platform::Linux:
            return PackagePlatform::Linux;
        case Utils::OS::Platform::MacOS:
            return PackagePlatform::MacOS;
        default:
            return PackagePlatform::Undefined;
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

    const char* Editor::SerializePackageModeValue(Editor::PackageMode package)
    {
        switch (package)
        {
        case PackageMode::Debug:
            return "Debug";
        case PackageMode::Release:
            return "Release";
        default:
            return "Undefined";
        }
    }

    const char* Editor::SerializePackageModeEnum()
    {
        return "Debug\0Release\0";
    }

    Editor::PackageManager::PackageManager()
    {
        
    }
#define DLL_NAME "GalaxyEngine"
#define BIN_NAME "GalaxyCore"

    static std::string s_xmakeContent = R"RAW(
add_rules("mode.debug", "mode.release")

if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
end

set_languages("c++20")

set_rundir("$(projectdir)")
set_targetdir("$(projectdir)")

-- Custom repo
add_repositories("galaxy-repo https://github.com/GalaxyEngine/xmake-repo")
add_requires("galaxymath")

add_defines("WITH_GAME")

if is_plat("windows") then
    add_defines("NOMINMAX")
end

target("%s")
    set_kind("shared")
    set_languages("c++20")

    add_includedirs("%s")
    add_includedirs("Generate/Headers")
    
    add_files("**.cpp")
    add_headerfiles("**.h")

    add_links("GalaxyEngine")
    
    add_packages("galaxymath")

    set_basename("Assembly")
    
    set_prefixname("")
target_end()
)RAW";

    
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
            
            if (!m_packagePath.has_value())
                return;
        }
        std::filesystem::path packagePath = m_packagePath.value();
        const std::filesystem::path assetPath = packagePath / ASSET_FOLDER_NAME;

        // Clear the package path

        //TODO : Copy only the assets used by the project ? (hard to tell if it's use in code)
        // Copy assets
        Utils::FileSystem::CopyFileTo(Resource::ResourceManager::GetAssetPath(), assetPath, std::filesystem::copy_options::recursive);
        // Copy resource assets
        Utils::FileSystem::CopyFileTo(ENGINE_RESOURCE_FOLDER_NAME, packagePath / ENGINE_RESOURCE_FOLDER_NAME, std::filesystem::copy_options::recursive);
        // Copy Generated code
        Path sourcePath = Resource::ResourceManager::GetProjectPath() / "Generate" / "Headers";
        std::filesystem::create_directories(packagePath / ENGINE_GENERATE_HEADER_PATH);
        Path destinationPath = packagePath / ENGINE_GENERATE_HEADER_PATH;
        Utils::FileSystem::CopyFileTo(sourcePath, destinationPath, std::filesystem::copy_options::recursive);

        std::filesystem::path exeFolder = GetBinFolder(m_packageMode);
        // copy the GalaxyGameDebug.exe into project name .exe
        Path fromBinPath = exeFolder / (std::string(BIN_NAME) + Utils::OS::GetBinaryExtension());
        Path toBinPath = (packagePath / Resource::ResourceManager::GetProjectPath().filename().stem()).generic_string() + Utils::OS::GetBinaryExtension();

        if (!std::filesystem::exists(fromBinPath))
        {
            PrintLog("Can't find the binary file need to compile in game mode");
            // TODO : Implement compilation of game mode
            return;
        }
            
        Utils::FileSystem::CopyFileTo(fromBinPath, toBinPath, std::filesystem::copy_options::overwrite_existing);

        //TODO Change the DLL name
        // copy the galaxy engine dll
        Path fromDLLPath = exeFolder / (std::string(DLL_NAME) + Utils::OS::GetDLLExtension());
        Path toDLLPath = (packagePath / DLL_NAME).string() + Utils::OS::GetDLLExtension();
        Utils::FileSystem::CopyFileTo(fromDLLPath, toDLLPath, std::filesystem::copy_options::overwrite_existing);
        
        // copy the lib dll
        Path fromLibPath = exeFolder / (DLL_NAME".lib");
        Path toLibPath = (packagePath / DLL_NAME).string() + ".lib";
        Utils::FileSystem::CopyFileTo(fromLibPath, toLibPath, std::filesystem::copy_options::overwrite_existing);

        Path fromSettingsPath = Resource::ResourceManager::GetProjectPath() / "project.settings";
        Path toSettingsPath = packagePath / "project.settings";
        Utils::FileSystem::CopyFileTo(fromSettingsPath, toSettingsPath, std::filesystem::copy_options::overwrite_existing);

        // Create xmake.lua
        std::ofstream xmakeFile(packagePath / "xmake.lua");

        // Get the filename without extension
        std::string filename = Resource::ResourceManager::GetProjectPath().filename().stem().string();

        // Prepare the buffer
        char xmakeContent[8192];
        std::strncpy(xmakeContent, s_xmakeContent.c_str(), sizeof(xmakeContent) - 1);
        xmakeContent[sizeof(xmakeContent) - 1] = '\0';  // Ensure null-termination

        // Get the project name
        std::string projectName = Resource::ResourceManager::GetProjectPath().filename().string();

        Path includePath = std::filesystem::current_path().parent_path() / "GalaxyEngine" / "include";
        
        // Format the content
        std::snprintf(xmakeContent, sizeof(xmakeContent), s_xmakeContent.c_str(),
            projectName.c_str(), includePath.generic_string().c_str());

        // Write to file
        xmakeFile << xmakeContent;
        xmakeFile.close();

        std::string command = "xmake f -p";

        switch (m_platform) {
        case PackagePlatform::Windows:
            command += "windows -a x64";
            break;
        case PackagePlatform::Linux:
            command += "linux";
            break;
        case PackagePlatform::MacOS:
            command += "macos";
            break;
        case PackagePlatform::Undefined:
            break;
        }

        switch (m_compiler) {
        case CompilerTool::MINGW:
            command = "xmake f -p mingw -a x86_64";
            break;
        case CompilerTool::Undefined:
            break;
        }

        switch (m_packageMode)
        {
        case PackageMode::Debug:
            command += " -m debug";
            break;
        case PackageMode::Release:
            command += " -m release";
            break;
        default: ;
        }

        // Package the project
        auto prevPath = std::filesystem::current_path();
        std::filesystem::current_path(packagePath);
        Utils::OS::RunCommand(command);
        Utils::OS::RunCommand("xmake");
        std::filesystem::current_path(prevPath);

        // Remove unnecessary files
        std::remove((packagePath / "xmake.lua").generic_string().c_str());
        std::filesystem::remove_all((packagePath / ".xmake").generic_string().c_str());
        std::filesystem::remove_all((packagePath / "build").generic_string().c_str());
        std::remove((packagePath / "Assembly.exp").generic_string().c_str());
        std::remove((packagePath / "Assembly.ilk").generic_string().c_str());
        std::remove((packagePath / "Assembly.pdb").generic_string().c_str());
        std::remove((packagePath / "compile.Assembly.pdb").generic_string().c_str());
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

            if (ImGui::BeginCombo("Package Type", SerializePackageModeValue(m_packageMode)))
            {
                for (int i = 0; i < static_cast<int>(PackageMode::Undefined); i++)
                {
                    const bool is_selected = (m_packageMode == static_cast<PackageMode>(i));
                    if (ImGui::Selectable(SerializePackageModeValue(static_cast<PackageMode>(i)), is_selected))
                        m_packageMode = static_cast<PackageMode>(i);
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
                ImGui::SetTooltip("Package mode used to package the project.");
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

    std::filesystem::path Editor::PackageManager::GetBinFolder(PackageMode type)
    {
        std::string platform;
        std::string arch;
        std::string mode;
#ifdef _WIN32
        platform = "windows";
#elif defined(__linux__)
        platform = "linux";
#endif
#if defined(_WIN64) || defined(__x86_64__) || defined(__amd64__)
        arch = "x64";
#else
        arch = "x86";
#endif
#ifdef  __MINGW32__
        platform = "mingw";
        arch = "x86_64";
#endif
        switch (type)
        {
        case PackageMode::Debug:
            mode = "gamedbg";
            break;
        case PackageMode::Release:
            mode = "game";
            break;
      break;      
        }
        return Path("../build") / platform / arch / mode;
    }

    Editor::PackagePlatform Editor::PackageManager::GetUserPlatform()
    {
        return PlatformToPackagePlatform(Utils::OS::GetPlatform());
    }
}
