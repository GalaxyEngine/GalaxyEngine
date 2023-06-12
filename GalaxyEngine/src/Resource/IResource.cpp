#include "pch.h"
#include "Resource/IResource.h"

Resource::IResource::IResource(const std::string& fullPath)
{
	p_type = IResource::GetTypeFromExtension(IResource::ExtractExtensionFromPath(fullPath));
	p_fullPath = fullPath;
	p_relativepath = IResource::ExtractRelativePathFromPath(fullPath);
	p_name = IResource::ExtractNameFromPath(fullPath);
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
	size_t pos = path.find("Assets");
	if (pos < path.size()) {
		return path.substr(pos);
	}
	else {
		PrintWarning("'Assets' not found in Resource %s", path.c_str());
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
