#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
#include "Type.h"

namespace GALAXY::Utils::FileSystem {
	
	std::fstream OpenFile(const Path& path);
	std::string ReadFile(const Path& path);
	std::ofstream GenerateFile(const Path& path);

	bool RemoveFile(const Path& path);

	bool CopyFileTo(const Path& sourcePath, const Path& destinationPath);

	void CopyFileTo(const Path& sourcePath, const Path& destinationPath, std::filesystem::copy_options options);

	bool FileExistNoExtension(const Path& path);

	Path ToLower(const Path& path);

	Path FindFileWithExtension(const Path& path, const std::string& extension);

	std::vector<Path> FindFilesWithExtension(const Path& path, const std::string& extension);
	
	Path FindFileWithNameInFolder(const Path& folderPath, const std::string& filename, bool extensionIncluded = false, bool searchInSubFolder = true);

}
