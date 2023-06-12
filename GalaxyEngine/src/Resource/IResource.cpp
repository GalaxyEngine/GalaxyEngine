#include "pch.h"
#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#define RESOURCE_FOLDER_NAME "assets"

Resource::IResource::IResource(const std::string& fullPath)
{
	p_fullPath = ResourceManager::StringToPath(fullPath);
	p_type = IResource::GetTypeFromExtension(IResource::ExtractExtensionFromPath(p_fullPath));
	p_relativepath = IResource::ExtractRelativePathFromPath(p_fullPath);
	p_name = IResource::ExtractNameFromPath(p_fullPath);
}

Resource::ResourceType Resource::IResource::GetTypeFromExtension(const std::string_view& ext)
{
	if (ext == ".jpg" || ext == ".png" || ext == ".jpeg")
		return ResourceType::Texture;
	else if (ext == ".obj" || ext == ".fbx")
		return ResourceType::Model;
	else
		return ResourceType::None;
}

std::string GALAXY::Resource::IResource::ExtractNameFromPath(std::string path, bool extension /*= true*/)
{
	size_t pos = path.find_last_of('\\');
	if (pos < path.size())
		path = path.substr(pos + 1);
	pos = path.find_last_of('/');
	if (pos < path.size())
		path = path.substr(pos + 1);
	if (!extension) {
		size_t extpos = path.find_last_of('.');
		path = path.substr(0, extpos);
	}
	return path;
}

std::string GALAXY::Resource::IResource::ExtractRelativePathFromPath(const std::string& path)
{
	size_t pos = path.find(RESOURCE_FOLDER_NAME);
	if (pos < path.size()) {
		return path.substr(pos);
	}
	else {
		PrintWarning("%s not found in Resource %s", RESOURCE_FOLDER_NAME, path.c_str());
		return "";
	}
}

std::string GALAXY::Resource::IResource::ExtractExtensionFromPath(const std::string& path)
{
	size_t pos = path.find_last_of('.');
	if (pos < path.size()) {
		return path.substr(pos);
	}
	else {
		PrintError("No extension found in Resource %s", path.c_str());
		return "";
	}
}
