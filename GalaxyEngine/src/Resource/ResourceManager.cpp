#include "pch.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"
#include "Resource/Model.h"

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
			ImportAllFilesInFolder(entry.path());
		}
		else
		{
			ImportResource(entry.path());
		}
	}
}

void Resource::ResourceManager::ImportResource(const std::filesystem::path& resourcePath)
{
	if (!resourcePath.has_extension())
		return;
	auto extension = resourcePath.extension();
	ResourceType type = Utils::FileInfo::GetTypeFromExtension(extension);
	switch (type)
	{
	case Resource::ResourceType::None:
		PrintError("Cannot Import resource with this extension : %s", extension.string().c_str());
		break;
	case Resource::ResourceType::Texture:
		// Default load all textures.
		GetOrLoad<Texture>(resourcePath);
		break;
	case Resource::ResourceType::Shader:
		AddResource(new Shader(resourcePath));
		break;
	case Resource::ResourceType::VertexShader:
		AddResource(new VertexShader(resourcePath));
		break;
	case Resource::ResourceType::FragmentShader:
		AddResource(new FragmentShader(resourcePath));
		break;
	case Resource::ResourceType::Model:
		//AddResource(new Model(resourcePath));
		GetOrLoad<Model>(resourcePath);
		break;
	case Resource::ResourceType::Mesh:
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

std::weak_ptr<Resource::Shader> Resource::ResourceManager::GetUnlitShader()
{
	std::string unlitPath = ENGINE_RESOURCE_FOLDER_NAME"\\shaders\\UnlitShader\\unlit.shader";
	if (m_resources.count(unlitPath))
		return std::dynamic_pointer_cast<Resource::Shader>(m_resources.at(unlitPath));
	return GetOrLoad<Resource::Shader>(unlitPath);
}

std::weak_ptr<Resource::Shader> Resource::ResourceManager::GetDefaultShader()
{
	return GetUnlitShader();
}

void Resource::ResourceManager::Release()
{
	m_instance.release();
}
