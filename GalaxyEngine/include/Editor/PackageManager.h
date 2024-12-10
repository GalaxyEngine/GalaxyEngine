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

        static PackagePlatform PlatformToPackagePlatform(Utils::OS::Platform platform);
        static const char* SerializePackagePlatformValue(PackagePlatform platform);
        static const char* SerializePackagePlatformEnum();

        enum class CompilerTool
        {
            MSVC = 0,
            GCC,
            MINGW,

            Undefined
        };

        CompilerTool GetUserCompiler();

        static const char* SerializeCompilerToolValue(CompilerTool tool);
        static const char* SerializeCompilerToolEnum();

        enum class PackageMode
        {
            Debug,
            Release,
            Undefined
        };
        
        static const char* SerializePackageModeValue(Editor::PackageMode package);
        static const char* SerializePackageModeEnum();

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
