#pragma once
#include "GalaxyAPI.h"
#include "Utils/FileSystem.h"
namespace GALAXY::Utils::FileSystem {
	std::fstream OpenFile(const std::filesystem::path& path);
	std::string ReadFile(const std::filesystem::path& path);
	std::ofstream GenerateFile(const std::filesystem::path& path);

	bool RemoveFile(const std::filesystem::path& path);

	bool FileExistNoExtension(const std::filesystem::path& path);
}
