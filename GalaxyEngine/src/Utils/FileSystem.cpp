#include "pch.h"
#include "Utils/FileSystem.h"

namespace GALAXY {
	std::fstream Utils::FileSystem::OpenFile(const std::filesystem::path& path)
	{
		std::fstream file(path);
		if (!file.is_open()) {
			PrintError("File %s could'nt be open", path.string().c_str());
		}
		return file;
	}

	std::string Utils::FileSystem::ReadFile(const std::filesystem::path& path)
	{
		std::ifstream in(path, std::ios::binary);
		if (in)
		{
			std::string contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();
			return(contents);
		}
		else
		{
			PrintError("Failed To Read %s", path.string().c_str());
			return std::string();
		}
	}

	std::ofstream Utils::FileSystem::GenerateFile(const std::filesystem::path& path)
	{
		std::ofstream outputFile(path, std::ios::trunc);
		if (outputFile.is_open()) {
			PrintLog("File created successfully : %s", path.string().c_str());
		}
		else {
			PrintError("Failed to create the file %s", path.string().c_str());
		}
		return outputFile;
	}

#ifndef _WIN32
#include <sys/stat.h>
#endif

	bool Utils::FileSystem::FileExistNoExtension(const std::filesystem::path& path)
	{
#ifdef _WIN32
		std::filesystem::path directory = path.parent_path();
		std::string filename = path.stem().string();
		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			if (entry.is_regular_file() && entry.path() != path && entry.path().stem().string() == filename) {
				return true;
			}
		}
		return false;
#else
		struct stat buffer;
		return (stat(path.string().c_str(), &buffer) == 0);
#endif
	}

	bool Utils::FileSystem::RemoveFile(const std::filesystem::path& path)
	{
		if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
			std::error_code ec;
			if (std::filesystem::remove(path, ec)) {
				PrintWarning("Remove file %s", path.string().c_str());
				return true;
			}
			else {
				PrintError("Failed to Remove file %s, err : %s", path.string().c_str(), ec.message().c_str());
				return false;
			}
		}
		return false;
	}

	bool Utils::FileSystem::CopyFileTo(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationPath)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::ifstream sourceFile(sourcePath, std::ios::binary);
		while (!sourceFile) { sourceFile = std::ifstream(sourcePath, std::ios::binary); }

		if (!sourceFile) {
			PrintError("File %s not found", sourcePath.string().c_str());
			return false;
		}

		std::ofstream destinationFile(destinationPath, std::ios::binary);

		if (!destinationFile) {
			PrintError("File %s not found", destinationPath.string().c_str());
			return false;
		}

		PrintLog("File %s Copied to %s", sourcePath.string().c_str(), destinationPath.string().c_str());

		destinationFile << sourceFile.rdbuf();

		sourceFile.close();
		destinationFile.close();

		return true;
	}

}