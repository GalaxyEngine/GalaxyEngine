#include "pch.h"
#include "Utils/FileSystem.h"

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
