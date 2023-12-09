#include "pch.h"

#include "Wrapper/MTLLoader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"
#include "Resource/PostProcessShader.h"
#include "Resource/Material.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Script.h"
#include "Resource/Scene.h"

#define AUTO_IMPORT
// Automatic import all model that not get a .gdata up to date

namespace GALAXY {
	std::unique_ptr<Resource::ResourceManager> Resource::ResourceManager::m_instance = nullptr;

	Resource::ResourceManager::~ResourceManager()
	{
		for (auto& val : m_resources | std::views::values)
		{
			val.reset();
		}
		m_resources.clear();
		for (auto& val : m_temporaryResources | std::views::values)
		{
			val.reset();
		}
		m_temporaryResources.clear();
	}

	void Resource::ResourceManager::ImportAllFilesInFolder(const Path& folder)
	{
		if (!std::filesystem::exists(folder))
			return;
		const std::filesystem::directory_iterator dirIt = std::filesystem::directory_iterator(folder);
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

	void Resource::ResourceManager::ImportResource(const Path& resourcePath)
	{
		// DO NOT TOUCH THIS
		if (!resourcePath.has_extension())
			return;
		const Path extension = resourcePath.extension();
		switch (ResourceType type = Utils::FileInfo::GetTypeFromExtension(extension))
		{
		case ResourceType::None:
			PrintError("Cannot Import resource with this extension : %s", extension.string().c_str());
			break;
		case ResourceType::Texture:
			// Default load all textures.
			AddResource<Texture>(resourcePath);
			break;
		case ResourceType::Shader:
			AddResource<Shader>(resourcePath);
			break;
		case ResourceType::PostProcessShader:
			AddResource<PostProcessShader>(resourcePath);
			break;
		case ResourceType::VertexShader:
			AddResource<VertexShader>(resourcePath);
			break;
		case ResourceType::FragmentShader:
			AddResource<FragmentShader>(resourcePath);
			break;
		case ResourceType::Material:
			AddResource<Material>(resourcePath);
			break;
		case ResourceType::Scene:
			AddResource<Scene>(resourcePath);
			break;
		case ResourceType::Model:
		{
#ifdef AUTO_IMPORT
			if (!IsDataFileUpToDate(resourcePath))
				GetOrLoad<Model>(resourcePath);
			else
				AddResource<Model>(resourcePath);
#else
			AddResource<Model>(resourcePath);
#endif
			break;
		}
		case ResourceType::Data:
		{
			const Path path = resourcePath.parent_path() / resourcePath.stem();
			if (!std::filesystem::exists(path)) {
				// remove if resource not exist buf data file exist
				Utils::FileSystem::RemoveFile(resourcePath);
				break;
			}
			break;
		}
		case ResourceType::Script:
		{
			// Default load all scripts
			GetOrLoad<Script>(resourcePath);
		}
		break;
		default:
			break;
		}
	}

	bool Resource::ResourceManager::IsDataFileUpToDate(const Path& resourcePath)
	{
		const Path dataFilePath = resourcePath.string() + ".gdata";

		const bool exist = std::filesystem::exists(dataFilePath);
		if (!exist)
			return false;

		const auto dataFileTime = std::filesystem::last_write_time(dataFilePath);
		const auto ResourceFileTime = std::filesystem::last_write_time(resourcePath);

		if (dataFileTime > ResourceFileTime)
			return true;
		return false;
	}

	void Resource::ResourceManager::Release()
	{
		m_instance.reset();
	}

}