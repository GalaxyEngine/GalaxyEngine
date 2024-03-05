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

#include <set>

#ifdef WITH_EDITOR
#include "Editor/ThumbnailCreator.h"
#include "Editor/UI/FileExplorer.h"
#include "Editor/UI/EditorUIManager.h"
#endif

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
			break;
		case ResourceType::Texture:
			// Default load all textures.
			AddResource<Texture>(resourcePath);
			break;
		case ResourceType::Shader:
			//#ifdef AUTO_IMPORT
						//if (!IsDataFileUpToDate(resourcePath))
			AddResource<Shader>(resourcePath);
			//else
				//AddResource<Shader>(resourcePath);
//#else
			//AddResource<Shader>(resourcePath);
//#endif
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
			//TODO Move to Needed Load method
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

	void Resource::ResourceManager::LoadNeededResources()
	{
		for (auto& resource : m_resources)
		{
			auto type = resource.second->GetFileInfo().GetResourceType();
			switch (type)
			{
			case ResourceType::None:
				break;
			case ResourceType::Texture:
				break;
			case ResourceType::Shader:
				if (auto shader = dynamic_pointer_cast<Shader>(resource.second))
				{
					GetOrLoad<Shader>(resource.first);
				}
				break;
			case ResourceType::PostProcessShader:
				break;
			case ResourceType::VertexShader:
				break;
			case ResourceType::GeometryShader:
				break;
			case ResourceType::FragmentShader:
				break;
			case ResourceType::Model:
#ifdef WITH_EDITOR
				if (!Editor::ThumbnailCreator::IsThumbnailUpToDate(resource.second.get()))
					GetOrLoad<Model>(resource.second->p_uuid);
#endif
				break;
			case ResourceType::Mesh:
				break;
			case ResourceType::Material:
#ifdef WITH_EDITOR
				if (!Editor::ThumbnailCreator::IsThumbnailUpToDate(resource.second.get()))
					GetOrLoad<Material>(resource.second->p_uuid);
#endif
				break;
			case ResourceType::Materials:
				break;
			case ResourceType::Data:
				break;
			case ResourceType::Script:
				break;
			case ResourceType::Scene:
				break;
			default:
				break;
			}
		}

#ifdef WITH_EDITOR
		if (!m_projectExists)
			return;
		const Path assetPath = this->GetAssetPath();

		m_fileWatchPrevious = std::make_shared<Editor::UI::File>(assetPath, true);
		m_fileWatchPrevious->FindAllChildren();

		Core::ThreadManager::GetInstance()->AddTask([this] { this->UpdateFileWatch(); });
#endif
	}

#ifdef WITH_EDITOR
	void Resource::ResourceManager::UpdateFileWatch()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		const Path assetPath = this->GetAssetPath();

		m_fileWatchCurrent = std::make_shared<Editor::UI::File>(assetPath, true);
		m_fileWatchCurrent->FindAllChildren();

		auto currentFiles = m_fileWatchCurrent->GetAllChildrenPath();
		auto previousFiles = m_fileWatchPrevious->GetAllChildrenPath();

		// Sort both lists to facilitate comparison
		std::sort(currentFiles.begin(), currentFiles.end());
		std::sort(previousFiles.begin(), previousFiles.end());

		// Find added files
		std::vector<std::filesystem::path> addedFiles;
		std::set_difference(currentFiles.begin(), currentFiles.end(),
			previousFiles.begin(), previousFiles.end(),
			std::back_inserter(addedFiles));

		// Find deleted files
		std::vector<std::filesystem::path> deletedFiles;
		std::set_difference(previousFiles.begin(), previousFiles.end(),
			currentFiles.begin(), currentFiles.end(),
			std::back_inserter(deletedFiles));

		bool shouldReload = false;
		// Print added files
		for (const auto& file : addedFiles) {
			shouldReload = true;
			PrintLog("Added file: %s", file.string().c_str());
			GetOrLoad(file);
		}

		// Print deleted files
		for (const auto& file : deletedFiles) {
			shouldReload = true;
			PrintLog("Remove file: %s", file.string().c_str());
			RemoveResource(file);
		}

		m_fileWatchPrevious = m_fileWatchCurrent;

		if (shouldReload)
			Editor::UI::EditorUIManager::GetInstance()->GetFileExplorer()->ReloadContent();

		if (!Core::ThreadManager::ShouldTerminate())
			UpdateFileWatch();
	}
#endif

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

	std::size_t HashContent(const std::string& content)
	{
		// For simplicity, we're just using the string's hash function here
		return std::hash<std::string>{}(content);
	}

	void Resource::ResourceManager::ReadCache()
	{
		/*
		* Check for every resources if it was on the previous launch
		* if no, check the it's a new resource
		*	and so check if a deleted file was with the same content
		* if yes continue
		 */
		if (!m_projectExists)
			return;
		const Path cachePath = this->GetProjectPath() / "Cache";
		CppSer::Parser parser(cachePath / "resource.cache");

		// Return if no cache or failed to open
		if (!parser.IsFileOpen())
			return;

		std::set<Path> newFiles;
		auto map = parser.GetValueMap()[0];

		for (auto& resource : m_resources)
		{
			if (resource.second->GetFileInfo().GetResourceDir() == ResourceDir::Editor || !std::filesystem::exists(resource.second->GetFileInfo().GetFullPath()))
			{
				map.erase(resource.first.string());
				continue;
			}
			if (map.contains(resource.first.string())) {

				map.erase(resource.first.string());
				continue;
			}
			newFiles.emplace(resource.second->GetFileInfo().GetFullPath());
			PrintLog("Detected new Resource %s", resource.first.string().c_str());
		}

		if (newFiles.empty() || map.empty())
			return;
		for (auto& newFile : newFiles)
		{
			auto content = Utils::FileSystem::ReadFile(newFile);
			if (!content.empty())
			{
				const uint64_t hash = HashContent(content);
				for (auto& deletedMap : map)
				{
					const Vec2<uint64_t> uuidHash = deletedMap.second.As<Vec2<uint64_t>>();
					if (uuidHash.y == hash)
					{
						PrintLog("File %s was rename in %s", newFile.string().c_str(), deletedMap.first.c_str());

						/* -- Handle rename --
						* Get UUID of the previous resource
						* Set it to the new by creating a .gdata file even if it already exist
						*/

						auto resource = GetResource<IResource>(newFile);
						resource.lock()->SetUUID(uuidHash.x);
						break;
					}
				}
			}
		}
	}

	void Resource::ResourceManager::CreateCache()
	{
		if (!m_projectExists)
			return;
		const Path cachePath = this->GetProjectPath() / "Cache";
		if (!std::filesystem::exists(cachePath))
			std::filesystem::create_directory(cachePath);

		CppSer::Serializer serializer(cachePath / "resource.cache");
		serializer << CppSer::Pair::BeginMap << "Resources";
		for (auto& val : m_resources | std::views::values)
		{
			if (val->GetFileInfo().GetResourceDir() == ResourceDir::Editor || !std::filesystem::exists(val->GetFileInfo().GetFullPath()))
				continue;

			auto content = Utils::FileSystem::ReadFile(val->GetFileInfo().GetFullPath());
			if (!content.empty())
			{
				const std::string path = val->GetFileInfo().GetRelativePath().string();
				const uint64_t hash = HashContent(content);
				const Vec2<uint64_t> uuidHash = { val->GetUUID(), hash };
				serializer << CppSer::Pair::Key << path << CppSer::Pair::Value << uuidHash;
			}
		}
		serializer << CppSer::Pair::EndMap << "Resources";
	}

	void Resource::ResourceManager::HandleRename(const Path& oldPath, const Path& newPath)
	{
		if (oldPath == newPath)
			return;

		std::filesystem::rename(oldPath, newPath);

		Path relativeOld = Utils::FileInfo::ToRelativePath(oldPath);
		Path relativeNew = Utils::FileInfo::ToRelativePath(newPath);
		std::set<Path> resourceKeys;

		for (auto& resource : m_instance->m_resources)
		{
			if (!resource.second || resource.second->GetFileInfo().GetResourceDir() == ResourceDir::Editor)
				continue;
			std::string relativeOldResource = resource.first.string();
			auto it = relativeOldResource.find(relativeOld.string());
			if (it != std::string::npos)
			{
				resourceKeys.emplace(resource.first);
			}
		}

		for (auto& resourceKey : resourceKeys)
		{
			const auto resource = m_instance->m_resources[resourceKey];

			Path oldResourcePath = resource->GetFileInfo().GetFullPath();
			Path newResourcePath = oldResourcePath;

			auto it = newResourcePath.string().find(oldPath.string());
			newResourcePath = newResourcePath.string().replace(it, oldPath.string().length(), newPath.string());

			if (exists(oldResourcePath))
				std::filesystem::rename(oldResourcePath, newResourcePath);

			// Rename .gdata file
			Path oldGDataFile = (resource->GetFileInfo().GetFullPath().string() + ".gdata").c_str();
			Path newGDataFile = (newResourcePath.string() + ".gdata").c_str();
			if (exists(oldGDataFile))
				std::filesystem::rename(oldGDataFile, newGDataFile);

			// Update file infos
			resource->p_fileInfo = Utils::FileInfo(newResourcePath);

			// Add to map the new key and erase the previous one
			m_instance->m_resources[resource->p_fileInfo.GetRelativePath()] = resource;
			m_instance->m_resources.erase(resourceKey);
		}
	}

	void Resource::ResourceManager::RenameSingle(const Path& oldPath, const Path& newPath)
	{
		auto relativeOld = Utils::FileInfo::ToRelativePath(oldPath);
		auto it = m_instance->m_resources.find(relativeOld);
		ASSERT(it != m_instance->m_resources.end());

		auto resource = it->second;
		resource->p_fileInfo = Utils::FileInfo(newPath);

		m_instance->m_resources.erase(it);
		m_instance->m_resources[resource->p_fileInfo.GetRelativePath()] = resource;
	}

	void Resource::ResourceManager::Release()
	{
		m_instance.reset();
	}

	Weak<GALAXY::Resource::IResource> Resource::ResourceManager::GetOrLoad(const Path& fullPath)
	{
		auto type = Utils::FileInfo::GetTypeFromExtension(fullPath.extension());
		switch (type)
		{
		case Resource::ResourceType::Shader:
			return GetOrLoad<Shader>(fullPath);
		case Resource::ResourceType::VertexShader:
			return GetOrLoad<VertexShader>(fullPath);
		case Resource::ResourceType::FragmentShader:
			return GetOrLoad<FragmentShader>(fullPath);
		case Resource::ResourceType::GeometryShader:
			return GetOrLoad<GeometryShader>(fullPath);
		case Resource::ResourceType::Materials:
			return GetOrLoad<Material>(fullPath);
		case Resource::ResourceType::PostProcessShader:
			return GetOrLoad<PostProcessShader>(fullPath);
		case Resource::ResourceType::Material:
			return GetOrLoad<Material>(fullPath);
		case Resource::ResourceType::Mesh:
			return GetOrLoad<Mesh>(fullPath);
		case Resource::ResourceType::Texture:
			return GetOrLoad<Texture>(fullPath);
		case Resource::ResourceType::Model:
			return GetOrLoad<Model>(fullPath);
		case Resource::ResourceType::Scene:
			return AddResource<Scene>(fullPath);
		case Resource::ResourceType::Script:
			return GetOrLoad<Script>(fullPath);
		case Resource::ResourceType::Data:
			return {};
		default:
			PrintWarning("Resource %s not handled", fullPath.string().c_str());
			return {};
		}
	}

}