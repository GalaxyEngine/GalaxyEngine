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
		Project,
		Editor,
		Both,
	};

	inline const char* SerializeResourceDirEnum()
	{
		return "Project\0Editor\0Both\0";
	}

	inline const char* SerializeResourceDirValue(ResourceDir space)
	{
		switch (space)
		{
		case ResourceDir::Editor:	return "Editor";
		case ResourceDir::Project:	return "Project";
		case ResourceDir::Both:	return "Both";
		default:			return "None";
		}
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
			FileInfo() = default;
			FileInfo(const Path& path, bool createRelativePath = true);
			~FileInfo() = default;

			static Path ToPath(const Path& path);
			static Path ToRelativePath(Path path);
			static Resource::ResourceType GetTypeFromExtension(const Path& ext);

			inline Path GetFullPath() const { return m_fullPath; }
			inline Path GetRelativePath() const { return m_relativePath; }
			inline String GetFileName() const { return m_fileName; }
			inline String GetFileNameNoExtension() const { return m_fullPath.filename().stem().string(); }
			inline Path GetExtension() const { return m_fullPath.empty() ? "" : m_fullPath.extension(); }
			inline ResourceDir GetResourceDir() const { return m_resourceDir; }
			inline Resource::ResourceType GetResourceType() const { return GetTypeFromExtension(GetExtension()); }

			inline bool isDirectory() const { return m_isDirectory; }
			inline bool Exist() const { return std::filesystem::exists(m_fullPath); }
		private:
			friend Resource::IResource;
			friend Resource::Mesh;
			friend Wrapper::OBJLoader;

			bool m_isDirectory = false;
			Path m_fullPath;
			Path m_relativePath;
			String m_fileName;
			ResourceDir m_resourceDir = ResourceDir::None;
		};
	}
}
#include "Utils/FileInfo.inl" 
