#include "pch.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Script.h"

#define AUTO_IMPORT_MODEL
// Automatic import all model that not get a .gdata

namespace GALAXY {
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
#ifndef AUTO_IMPORT_MODEL
			AddResource(new Model(resourcePath));
#else
			{
				std::filesystem::path dataFilePath = resourcePath.parent_path() / resourcePath.stem();
				if (!std::filesystem::exists(dataFilePath.wstring() + L".gdata"))
					GetOrLoad<Model>(resourcePath);
				else
					AddResource(new Model(resourcePath));
			}
#endif
			break;
		case Resource::ResourceType::Data:
		{
			if (!Utils::FileSystem::FileExistNoExtension(resourcePath)) {
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

	void Resource::ResourceManager::ProcessDataFile(const std::filesystem::path& dataPath)
	{
		auto& file = Utils::FileSystem::OpenFile(dataPath);
		std::string line;
		bool process = false;
		ResourceType type;
		std::filesystem::path originFilePath;

		while (std::getline(file, line)) 
		{
			std::istringstream iss(line);
			std::string token;
			iss >> token;
			if (token == "Origin:")
			{
				process = true;
				iss >> originFilePath;
				type = Utils::FileInfo::GetTypeFromExtension(originFilePath.extension());
			}
			else if (process)
			{
				switch (type)
				{
				case Resource::ResourceType::Model: 
				{
					auto meshPath = Mesh::CreateMeshPath(originFilePath, token);
					AddResource(new Mesh(meshPath));
					break;
				}
				}
			}
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

}