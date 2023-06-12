#include "pch.h"
#include "Resource/ResourcesManager.h"
#include "Resource/Texture.h"
Resource::ResourceManager Resource::ResourceManager::m_instance;

Resource::ResourceManager::~ResourceManager()
{
}

void GALAXY::Resource::ResourceManager::ImportAllFilesInFolder(const std::filesystem::path& folder)
{

}

void GALAXY::Resource::ResourceManager::ImportResource(const std::filesystem::path& resourcePath)
{
	std::string extension = IResource::ExtractExtensionFromPath(resourcePath.string());
	ResourceType type = IResource::GetTypeFromExtension(extension);
	switch (type)
	{
	case GALAXY::Resource::ResourceType::None:
		break;
	case GALAXY::Resource::ResourceType::Texture:
		AddResource(new Texture(resourcePath.string()));
		break;
	case GALAXY::Resource::ResourceType::Shader:
		break;
	case GALAXY::Resource::ResourceType::Model:
		break;
	case GALAXY::Resource::ResourceType::Mesh:
		break;
	default:
		break;
	}
}

