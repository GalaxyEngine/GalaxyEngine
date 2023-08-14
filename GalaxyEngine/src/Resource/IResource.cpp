#include "pch.h"
#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#include "Core/Application.h"

Resource::IResource::IResource(const std::string& fullPath)
{
	p_fullPath = ResourceManager::StringToPath(fullPath);
	p_type = IResource::GetTypeFromExtension(IResource::ExtractExtensionFromPath(p_fullPath));
	p_relativePath = IResource::ExtractRelativePathFromPath(p_fullPath);
	p_name = IResource::ExtractNameFromPath(p_fullPath);
}

Resource::ResourceType Resource::IResource::GetTypeFromExtension(const std::string_view& ext)
{
	if (ext == ".jpg" || ext == ".png" || ext == ".jpeg")
		return ResourceType::Texture;
	else if (ext == ".obj" || ext == ".fbx")
		return ResourceType::Model;
	else if (ext == ".shader")
		return ResourceType::Shader;
	else if (ext == ".vert" || ext == "vs" || ext == "vsh" || ext == "glslv")
		return ResourceType::VertexShader;
	else if (ext == ".geom" || ext == "gs" || ext == "gsh" || ext == "glslg")
		return ResourceType::GeometryShader;
	else if (ext == ".frag" || ext == "fs" || ext == "fsh" || ext == "glslf")
		return ResourceType::FragmentShader;
	else if (ext == ".mat")
		return ResourceType::Material;
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
	else if (path.size() > static_cast<std::string>(ENGINE_RESOURCE_FOLDER_NAME).length()) {
		auto startWith = path.substr(0, static_cast<std::string>(ENGINE_RESOURCE_FOLDER_NAME).length());
		if (startWith == ENGINE_RESOURCE_FOLDER_NAME)
			return path;
	}
	PrintWarning("%s not found in Resource %s", RESOURCE_FOLDER_NAME, path.c_str());
	return "";
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

void GALAXY::Resource::IResource::SendRequest()
{
	Core::ThreadManager::GetInstance()->Lock();
	Core::Application::GetInstance().AddResourceToSend(p_fullPath);
	Core::ThreadManager::GetInstance()->Unlock();
}
