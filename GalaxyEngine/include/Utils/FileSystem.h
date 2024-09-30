#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
namespace GALAXY::Utils::FileSystem {
	std::fstream OpenFile(const std::filesystem::path& path);
	std::string ReadFile(const std::filesystem::path& path);
	std::ofstream GenerateFile(const std::filesystem::path& path);

	bool RemoveFile(const std::filesystem::path& path);

	bool CopyFileTo(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationPath);

	bool FileExistNoExtension(const std::filesystem::path& path);

	std::filesystem::path ToLower(const std::filesystem::path& path);
}
