#include "pch.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"

std::unique_ptr<Resource::ResourceManager> Resource::ResourceManager::m_instance = nullptr;

Resource::ResourceManager::~ResourceManager()
{
}

void Resource::ResourceManager::ImportAllFilesInFolder(const std::filesystem::path& folder)
{
	if (!std::filesystem::exists(folder))
		return;
	auto dirIt = std::filesystem::directory_iterator(folder);
	for (const std::filesystem::directory_entry& entry : dirIt) {
		if (entry.is_directory())
		{
			ImportAllFilesInFolder(entry.path().string());
		}
		else
		{
			ImportResource(entry.path().string());
		}
	}
}

void Resource::ResourceManager::ImportResource(const std::string& resourcePath)
{
	std::string extension = IResource::ExtractExtensionFromPath(resourcePath);
	ResourceType type = IResource::GetTypeFromExtension(extension);
	switch (type)
	{
	case GALAXY::Resource::ResourceType::None:
		PrintError("Cannot Import resource with this extension : %s", extension.c_str());
		break;
	case GALAXY::Resource::ResourceType::Texture:
		GetOrLoad<Texture>(resourcePath);
		break;
	case GALAXY::Resource::ResourceType::Shader:
		AddResource(new Shader(resourcePath));
		break;
	case GALAXY::Resource::ResourceType::VertexShader:
		AddResource(new VertexShader(resourcePath));
		break;
	case GALAXY::Resource::ResourceType::FragmentShader:
		AddResource(new FragmentShader(resourcePath));
		break;
	case GALAXY::Resource::ResourceType::Model:
		break;
	case GALAXY::Resource::ResourceType::Mesh:
		break;
	default:
		break;
	}
}

Resource::ResourceManager* Resource::ResourceManager::GetInstance()
{
	if (m_instance == nullptr) {
		m_instance = std::make_unique<ResourceManager>();
	}
	return m_instance.get();
}

std::string Resource::ResourceManager::StringToRelativePath(const std::string& value)
{
	std::string result = StringToPath(value);
	result = IResource::ExtractRelativePathFromPath(result);

	return result;
}

std::string GALAXY::Resource::ResourceManager::StringToPath(const std::string& value)
{
	std::string result = value;

	// Convert to lowercase
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);

	// Replace '/' with '\\'
	std::replace(result.begin(), result.end(), '/', '\\');

	return result;
}
