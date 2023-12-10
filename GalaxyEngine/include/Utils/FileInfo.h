#pragma once
#include "GalaxyAPI.h"
#include <filesystem>

using Path = std::filesystem::path;

namespace GALAXY 
{
	namespace Resource
	{
		class IResource;
		class Mesh;
		enum class ResourceType;
	}
	enum class ResourceDir
	{
		None,
		Editor,
		Project,
	};
	namespace Wrapper
	{
		class OBJLoader;
	}
	namespace Utils
	{
		class FileInfo
		{
		public:
			FileInfo() = default;
			explicit FileInfo(const Path& path, bool createRelativePath = true);
			~FileInfo() = default;

			static Path ToPath(const Path& path);
			static Path ToRelativePath(Path path);
			static Resource::ResourceType GetTypeFromExtension(const Path& ext);

			inline Path GetFullPath() const { return m_fullPath; }
			inline Path GetRelativePath() const { return m_relativePath; }
			inline String GetFileName() const { return m_fileName; }
			inline String GetFileNameNoExtension() const { return m_fileNameNoExtension; }
			inline Path GetExtension() const { return m_extension; }
			inline Resource::ResourceType GetResourceType() const { return m_resourceType; }
			inline ResourceDir GetResourceDir() const { return m_resourceDir; }

			inline bool isDirectory() const { return std::filesystem::is_directory(m_fullPath); }
			inline bool Exist() const { return std::filesystem::exists(m_fullPath); }
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
			ResourceDir m_resourceDir = ResourceDir::None;

			Resource::ResourceType m_resourceType = static_cast<Resource::ResourceType>(0);
		};
	}
}
#include "Utils/FileInfo.inl" 
