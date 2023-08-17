#pragma once
#include "GalaxyAPI.h"
#include <filesystem>

#define ASSET_FOLDER_NAME "Assets"
#define ENGINE_RESOURCE_FOLDER_NAME "CoreResources"

namespace GALAXY 
{
	namespace Resource
	{
		class IResource;
		enum class ResourceType;
	}
	namespace Wrapper
	{
		class OBJLoader;
	}
	namespace Utils
	{
		class FileInfo
		{
		public:
			FileInfo() {}
			FileInfo(const std::filesystem::path& path);
			~FileInfo() {}

			static std::filesystem::path ToPath(std::filesystem::path path);
			static std::filesystem::path ToRelativePath(std::filesystem::path path);
			static Resource::ResourceType GetTypeFromExtension(const std::filesystem::path& ext);

			std::filesystem::path GetFullPath() const { return m_fullPath; }
			std::filesystem::path GetRelativePath() const { return m_relativePath; }
			std::filesystem::path GetFileName() const { return m_fileName; }
			std::filesystem::path GetExtension() const { return m_extension; }
			Resource::ResourceType GetResourceType() const { return m_resouceType; }

			bool isDirectory() { return std::filesystem::is_directory(m_fullPath); }
		private:
			friend Resource::IResource;
			friend Wrapper::OBJLoader;
			std::filesystem::path m_fullPath;
			std::filesystem::path m_relativePath;
			std::filesystem::path m_fileName;
			std::filesystem::path m_extension;
			bool m_exist = false;

			Resource::ResourceType m_resouceType;
		};
	}
}
#include "Utils/FileInfo.inl" 
