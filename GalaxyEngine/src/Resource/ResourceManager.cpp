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
// Automatic import all model that not get a .gdata

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
	void ImportResource(const Path& resourcePath)
	{
		/*
		auto resource = AddResource<T>(resourcePath);
		auto result = resource.ParseData();
		// Check for .gdata file
		if (result == SUCCESS)
		{
			// .gdata opened and parsed success
		}
		else if (result == FAILED)
		{
			// create the .gdata file with only uuid
		}
		else if (result == PARTIAL)
		{
			// Need to load the resource because of either:
			// 1. Need to be load at start
			// 2. .gdata file is out of data and the resource is a model
			// 3.
		}
		*/
	}

	void Resource::ResourceManager::ImportResource(const Path& resourcePath)
	{
		// DO NOT TOUCH THIS
		if (!resourcePath.has_extension())
			return;
		const Path extension = resourcePath.extension();
		switch (ResourceType type = Utils::FileInfo::GetTypeFromExtension(extension))
		{
		case Resource::ResourceType::None:
			PrintError("Cannot Import resource with this extension : %s", extension.string().c_str());
			break;
		case Resource::ResourceType::Texture:
			// Default load all textures.
			GetOrLoad<Texture>(resourcePath);
			break;
		case Resource::ResourceType::Shader:
			AddResource<Shader>(resourcePath);
			break;
		case Resource::ResourceType::PostProcessShader:
			AddResource<PostProcessShader>(resourcePath);
			break;
		case Resource::ResourceType::VertexShader:
			AddResource<VertexShader>(resourcePath);
			break;
		case Resource::ResourceType::FragmentShader:
			AddResource<FragmentShader>(resourcePath);
			break;
		case Resource::ResourceType::Material:
			AddResource<Material>(resourcePath);
			break;
		case Resource::ResourceType::Scene:
			AddResource<Scene>(resourcePath);
			break;
		case Resource::ResourceType::Model:
		{
#ifdef AUTO_IMPORT
			if (!CheckForDataFile(resourcePath))
				GetOrLoad<Model>(resourcePath);
			else
				AddResource<Model>(resourcePath);
#else
			AddResource<Model>(resourcePath);
#endif
			break;
		}
		case Resource::ResourceType::Data:
		{
			const Path path = resourcePath.parent_path() / resourcePath.stem();
			if (!std::filesystem::exists(path)) {
				// remove if resource not exist buf data file exist
				Utils::FileSystem::RemoveFile(resourcePath);
				break;
			}
			ProcessDataFile(resourcePath);
			break;
		}
		case Resource::ResourceType::Script:
		{
			// Default load all scripts
			GetOrLoad<Resource::Script>(resourcePath);
		}
		break;
		default:
			break;
		}
	}

	bool Resource::ResourceManager::CheckForDataFile(const Path& resourcePath)
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

	void Resource::ResourceManager::ProcessDataFile(const Path& dataPath)
	{
		// TODO : only check if the resource still exist
		// Do the parsing in a new method ParseData
		std::fstream file = Utils::FileSystem::OpenFile(dataPath);
		std::string line;
		bool process = false;
		ResourceType type;
		Path originFilePath;

		while (std::getline(file, line))
		{
			std::istringstream iss(line);
			std::string token;
			iss >> token;
			if (token == "Origin:")
			{
				process = true;
				iss >> originFilePath;
				type = Utils::FileInfo::GetTypeFromExtension(dataPath.stem().extension());
			}
			else if (process)
			{
				switch (type)
				{
				case Resource::ResourceType::Model:
				{
					auto meshPath = Mesh::CreateMeshPath(originFilePath, token);
					AddResource<Mesh>(meshPath);
					break;
				}
				default :
					break;
				}
			}
		}
	}

	void Resource::ResourceManager::Release()
	{
		m_instance.reset();
	}

}