#pragma once
#include "GalaxyAPI.h"

#include <optional>
#include <filesystem>

namespace GALAXY 
{
    namespace Utils::OS
    {
        enum class Platform;
    }
    namespace Editor
    {
        enum class PackagePlatform
        {
            Windows = 0,
            Linux,
            MacOS,
            Undefined
        };

        inline const char* to_string(PackagePlatform e)
        {
            switch (e)
            {
            case PackagePlatform::Windows: return "Windows";
            case PackagePlatform::Linux: return "Linux";
            case PackagePlatform::MacOS: return "MacOS";
            case PackagePlatform::Undefined: return "Undefined";
            default: return "unknown";
            }
        }

        PackagePlatform PlatformToPackagePlatform(Utils::OS::Platform platform);
        const char* SerializePackagePlatformEnum();

        enum class CompilerTool
        {
            MSVC = 0,
            GCC,
            MINGW,
            CLANG,
            APPLE_CLANG,
            INTEL,

            Undefined
        };

        inline const char* to_string(CompilerTool e)
        {
            switch (e)
            {
            case CompilerTool::MSVC: return "MSVC";
            case CompilerTool::GCC: return "GCC";
            case CompilerTool::MINGW: return "MINGW";
            case CompilerTool::CLANG: return "CLANG";
            case CompilerTool::APPLE_CLANG: return "APPLE_CLANG";
            case CompilerTool::INTEL: return "INTEL";
            case CompilerTool::Undefined: return "Undefined";
            default: return "unknown";
            }
        }

        CompilerTool GetUserCompiler();

        const char* SerializeCompilerToolEnum();

        enum class PackageMode
        {
            Debug,
            Release,
            Undefined
        };

        inline const char* to_string(PackageMode e)
        {
            switch (e)
            {
            case PackageMode::Debug: return "Debug";
            case PackageMode::Release: return "Release";
            case PackageMode::Undefined: return "Undefined";
            default: return "unknown";
            }
        }
        
        const char* SerializePackageModeEnum();

        class PackageManager
        {
        public:
            PackageManager();
            ~PackageManager() = default;

            void PackageProject(bool forceSetBuildPath = false);
            
            void DrawSettings();

            void OpenSettings();

            bool SetPackagePath(const std::filesystem::path& path);

            static std::filesystem::path GetBinFolder(PackageMode type = PackageMode::Debug);

        private:
            PackagePlatform GetUserPlatform();
        private:
            PackagePlatform m_platform = PackagePlatform::Windows;
            CompilerTool m_compiler = CompilerTool::MSVC;
            PackageMode m_packageMode = PackageMode::Debug;

            std::optional<std::filesystem::path> m_packagePath;
        };
    }
}
