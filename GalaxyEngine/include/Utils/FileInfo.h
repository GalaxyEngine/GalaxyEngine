#pragma once
#include "GalaxyAPI.h"
#include <filesystem>

using Path = std::filesystem::path;

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
			FileInfo(const Path& path, bool createRelativePath = true);
			~FileInfo() {}

			static Path ToPath(Path path);
			static Path ToRelativePath(Path path);
			static Resource::ResourceType GetTypeFromExtension(const Path& ext);

			inline Path GetFullPath() const { return m_fullPath; }
			inline Path GetRelativePath() const { return m_relativePath; }
			inline String GetFileName() const { return m_fileName; }
			inline String GetFileNameNoExtension() const { return m_fileNameNoExtension; }
			inline Path GetExtension() const { return m_extension; }
			inline Resource::ResourceType GetResourceType() const { return m_resourceType; }

			inline bool isDirectory() { return std::filesystem::is_directory(m_fullPath); }
		private:
			friend Resource::IResource;
			friend Resource::Mesh;
			friend Wrapper::OBJLoader;

			Path m_fullPath;
			Path m_relativePath;
			String m_fileName;
			String m_fileNameNoExtension;
			Path m_extension;
			bool m_exist = false;

			Resource::ResourceType m_resourceType = (Resource::ResourceType)0;
		};
	}
}
#include "Utils/FileInfo.inl" 
