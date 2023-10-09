#include "pch.h"
#include "Utils/FileInfo.h"

#include "Resource/ResourceManager.h"
#include "Resource/IResource.h"
namespace GALAXY
{
	Utils::FileInfo::FileInfo(const std::filesystem::path& path)
	{
		m_exist = std::filesystem::exists(path);

		m_fullPath = ToPath(path);
		m_relativePath = ToRelativePath(m_fullPath);
		m_fileName = m_fullPath.filename().string();
		m_fileNameNoExtension = m_fileName.substr(0, m_fileName.find_last_of('.'));
		m_extension = m_relativePath.extension();
		m_resourceType = GetTypeFromExtension(m_extension);

		if (!m_exist)
		{
			PrintWarning("File %s does not exist", path.string().c_str());
		}
	}

	std::filesystem::path Utils::FileInfo::ToPath(std::filesystem::path path)
	{
		std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
		// Temp Try to remove this line :
		// return canonicalPath.make_preferred();
		return canonicalPath;
	}

	std::filesystem::path Utils::FileInfo::ToRelativePath(std::filesystem::path path)
	{
		path = ToPath(path);
		if (auto relative = std::filesystem::relative(path, Resource::ResourceManager::GetInstance()->GetAssetPath()); !relative.empty() && relative.string().find(ENGINE_RESOURCE_FOLDER_NAME) == std::string::npos)
		{
			if (relative == ".")
				return ASSET_FOLDER_NAME;
			return ASSET_FOLDER_NAME / relative;
		}
		else if (auto relative = std::filesystem::relative(path, ENGINE_RESOURCE_FOLDER_NAME); !relative.empty())
		{
			return ENGINE_RESOURCE_FOLDER_NAME / relative;
		}
		else
		{
			PrintWarning("%s file is not inside the asset folder, no relative path created", path.string().c_str());
			return path;
		}
	}

	Resource::ResourceType Utils::FileInfo::GetTypeFromExtension(const std::filesystem::path& ext)
	{
		using namespace Resource;
		if (ext == ".jpg" || ext == ".png" || ext == ".jpeg")
			return ResourceType::Texture;
		else if (ext == ".obj" || ext == ".fbx")
			return ResourceType::Model;
		else if (ext == ".shader")
			return ResourceType::Shader;
		else if (ext == ".ppshader")
			return ResourceType::PostProcessShader;
		else if (ext == ".vert" || ext == "vs" || ext == "vsh" || ext == "glslv")
			return ResourceType::VertexShader;
		else if (ext == ".geom" || ext == "gs" || ext == "gsh" || ext == "glslg")
			return ResourceType::GeometryShader;
		else if (ext == ".frag" || ext == "fs" || ext == "fsh" || ext == "glslf")
			return ResourceType::FragmentShader;
		else if (ext == ".mat")
			return ResourceType::Material;
		else if (ext == ".gdata")
			return ResourceType::Data;
		else if (ext == ".h" || ext == ".hpp" || ext == ".cpp" || ext == "cc")
			return ResourceType::Script;
		else
			return ResourceType::None;
	}
}