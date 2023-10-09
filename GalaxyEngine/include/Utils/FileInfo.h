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
		class Mesh;
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

			inline std::filesystem::path GetFullPath() const { return m_fullPath; }
			inline std::filesystem::path GetRelativePath() const { return m_relativePath; }
			inline std::string GetFileName() const { return m_fileName; }
			inline std::string GetFileNameNoExtension() const { return m_fileNameNoExtension; }
			inline std::filesystem::path GetExtension() const { return m_extension; }
			inline Resource::ResourceType GetResourceType() const { return m_resourceType; }

			inline bool isDirectory() { return std::filesystem::is_directory(m_fullPath); }
		private:
			friend Resource::IResource;
			friend Resource::Mesh;
			friend Wrapper::OBJLoader;

			std::filesystem::path m_fullPath;
			std::filesystem::path m_relativePath;
			std::string m_fileName;
			std::string m_fileNameNoExtension;
			std::filesystem::path m_extension;
			bool m_exist = false;

			Resource::ResourceType m_resourceType = (Resource::ResourceType)0;
		};
	}
}
#include "Utils/FileInfo.inl" 
