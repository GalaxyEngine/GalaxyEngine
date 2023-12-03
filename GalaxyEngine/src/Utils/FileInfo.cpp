#include "pch.h"
#include "Utils/FileInfo.h"

#include "Resource/ResourceManager.h"
#include "Resource/IResource.h"

std::unordered_map<std::string, Resource::ResourceType> resourceFromExtensionMap =
{
	{".jpg",		Resource::ResourceType::Texture},
	{".png",		Resource::ResourceType::Texture},
	{".jpeg",		Resource::ResourceType::Texture},
	{".obj",		Resource::ResourceType::Model},
	{".fbx",		Resource::ResourceType::Model},
	{".shader",		Resource::ResourceType::Shader},
	{".vert",		Resource::ResourceType::VertexShader},
	{".vs",			Resource::ResourceType::VertexShader},
	{".vsh",		Resource::ResourceType::VertexShader},
	{".glslv",		Resource::ResourceType::VertexShader},
	{".frag",		Resource::ResourceType::FragmentShader},
	{".fs",			Resource::ResourceType::FragmentShader},
	{".fsh",		Resource::ResourceType::FragmentShader},
	{".glslf",		Resource::ResourceType::FragmentShader},
	{".geom",		Resource::ResourceType::GeometryShader},
	{".gs",			Resource::ResourceType::GeometryShader},
	{".gsh",		Resource::ResourceType::GeometryShader},
	{".glslg",		Resource::ResourceType::GeometryShader},
	{".mat",		Resource::ResourceType::Material},
	{".mtl",		Resource::ResourceType::Materials},
	{".gdata",		Resource::ResourceType::Data},
	{".h",			Resource::ResourceType::Script},
	{".hpp",		Resource::ResourceType::Script},
	{".cpp",		Resource::ResourceType::Script},
	{".cc",			Resource::ResourceType::Script},
	{".galaxy",		Resource::ResourceType::Scene},
	{".ppshader",	Resource::ResourceType::PostProcessShader},
};

namespace GALAXY
{
	Utils::FileInfo::FileInfo(const Path& path, bool createRelativePath/* = true*/)
	{
		createRelativePath |= path == NONE_RESOURCE;
		m_exist = std::filesystem::exists(path);

		m_fullPath = ToPath(path);
		if (createRelativePath)
			m_relativePath = ToRelativePath(m_fullPath);
		m_fileName = m_fullPath.filename().string();
		m_fileNameNoExtension = m_fileName.substr(0, m_fileName.find_last_of('.'));
		m_extension = m_fullPath.extension();
		m_resourceType = GetTypeFromExtension(m_extension);

		if (!m_exist)
		{
			PrintWarning("File %s does not exist", path.string().c_str());
		}
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
		if (const Path relative = std::filesystem::relative(path, Resource::ResourceManager::GetInstance()->GetAssetPath()); !relative.empty() && relative.string().find(ENGINE_RESOURCE_FOLDER_NAME) == std::string::npos)
		{
			if (relative == ".")
				return ASSET_FOLDER_NAME;
			return ASSET_FOLDER_NAME / relative;
		}
		else if (const Path relative = std::filesystem::relative(path, ENGINE_RESOURCE_FOLDER_NAME); !relative.empty())
		{
			return ENGINE_RESOURCE_FOLDER_NAME / relative;
		}
		else
		{
			PrintWarning("%s file is not inside the asset folder, no relative path created", path.string().c_str());
			return path;
		}
	}

	Resource::ResourceType Utils::FileInfo::GetTypeFromExtension(const Path& ext)
	{
		if (resourceFromExtensionMap.contains(ext.string()))
			return resourceFromExtensionMap[ext.string()];
		return Resource::ResourceType::None;
	}
}