#pragma once
#include "GalaxyAPI.h"
#include "Utils/FileSystem.h"
namespace GALAXY::Utils::FileSystem {
	std::fstream OpenFile(const std::filesystem::path& path);
	std::string ReadFile(const std::filesystem::path& path);
}
