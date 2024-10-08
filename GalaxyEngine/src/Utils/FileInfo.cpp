#include "pch.h"
#include "Utils/FileInfo.h"

#include "Resource/ResourceManager.h"
#include "Resource/IResource.h"

std::unordered_map<std::string, Resource::ResourceType> resourceFromExtensionMap =
{
	{".jpg",		Resource::ResourceType::Texture},
	{".png",		Resource::ResourceType::Texture},
	{".jpeg",		Resource::ResourceType::Texture},
	{".tga",		Resource::ResourceType::Texture},
	{".obj",		Resource::ResourceType::Model},
	{".fbx",		Resource::ResourceType::Model},
	{".shader",	Resource::ResourceType::Shader},
	{".vert",		Resource::ResourceType::VertexShader},
	{".vs",		Resource::ResourceType::VertexShader},
	{".vsh",		Resource::ResourceType::VertexShader},
	{".glslv",	Resource::ResourceType::VertexShader},
	{".frag",		Resource::ResourceType::FragmentShader},
	{".fs",		Resource::ResourceType::FragmentShader},
	{".fsh",		Resource::ResourceType::FragmentShader},
	{".glslf",	Resource::ResourceType::FragmentShader},
	{".geom",		Resource::ResourceType::GeometryShader},
	{".gs",		Resource::ResourceType::GeometryShader},
	{".gsh",		Resource::ResourceType::GeometryShader},
	{".glslg",	Resource::ResourceType::GeometryShader},
	{".mat",		Resource::ResourceType::Material},
	{".mtl",		Resource::ResourceType::Materials},
	{".gdata",	Resource::ResourceType::Data},
	{".h",		Resource::ResourceType::Script},
	{".hpp",		Resource::ResourceType::Script},
	{".cpp",		Resource::ResourceType::Script},
	{".cc",		Resource::ResourceType::Script},
	{".galaxy",	Resource::ResourceType::Scene},
	{".ppshader",	Resource::ResourceType::PostProcessShader},
	{".mp3",		Resource::ResourceType::Sound},
	{".wav",		Resource::ResourceType::Sound},
	{".flac",		Resource::ResourceType::Sound},
	{".prefab", 	Resource::ResourceType::Prefab}

};

namespace GALAXY
{
	Utils::FileInfo::FileInfo(const Path& path, bool createRelativePath/* = true*/)
	{
		createRelativePath |= path == NONE_RESOURCE;

		m_fullPath = ToPath(path);
		if (createRelativePath)
			m_relativePath = ToRelativePath(m_fullPath);

		if (!m_relativePath.empty())
		{
			if (m_relativePath.string().find(ENGINE_RESOURCE_FOLDER_NAME) != std::string::npos)
			{
				m_resourceDir = ResourceDir::Editor;
			}
			else if (m_relativePath.string().find(ASSET_FOLDER_NAME) != std::string::npos)
			{
				m_resourceDir = ResourceDir::Project;
			}
		}

		m_isDirectory = std::filesystem::is_directory(m_fullPath);

		m_resourceType = GetTypeFromExtension(m_fullPath.extension());
	}

	Path Utils::FileInfo::ToPath(const Path& path)
	{
		Path canonicalPath = std::filesystem::weakly_canonical(path);
		// Temp Try to remove this line :
		// return canonicalPath.make_preferred();
		return canonicalPath;
	}

	Path Utils::FileInfo::ToRelativePath(Path path)
	{
		path = ToPath(path);
		const Path relativeAsset = std::filesystem::relative(path, Resource::ResourceManager::GetInstance()->GetAssetPath());
		if (!relativeAsset.empty() && relativeAsset.string().find(ENGINE_RESOURCE_FOLDER_NAME) == std::string::npos)
		{
			if (relativeAsset == ".")
				return ASSET_FOLDER_NAME;
			return ASSET_FOLDER_NAME / relativeAsset;
		}
		const Path relativeEngine = std::filesystem::relative(path, ENGINE_RESOURCE_FOLDER_NAME);
		if (!relativeEngine.empty())
		{
			return ENGINE_RESOURCE_FOLDER_NAME / relativeEngine;
		}
		return path;
	}

	Resource::ResourceType Utils::FileInfo::GetTypeFromExtension(const Path& ext)
	{
		if (resourceFromExtensionMap.contains(ext.string()))
			return resourceFromExtensionMap[ext.string()];
		return Resource::ResourceType::None;
	}
}