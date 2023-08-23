#pragma once
#include "Resource/ResourceManager.h"
namespace GALAXY
{
	void Resource::ResourceManager::AddResource(IResource* resource)
	{
		m_resources[resource->GetFileInfo().GetRelativePath()] = std::shared_ptr<IResource>(resource);
	}

	void Resource::ResourceManager::AddResource(const std::shared_ptr<IResource>& resource)
	{
		m_resources[resource->GetFileInfo().GetRelativePath()] = resource;
	}

	void Resource::ResourceManager::RemoveResource(IResource* resource)
	{
		if (auto it = m_resources.find(resource->GetFileInfo().GetRelativePath());  it != m_resources.end())
		{
			it->second.reset();
			m_resources.erase(it);
		}
		else
		{
			PrintError("Resource %s not found in Resource Manager", resource->GetFileInfo().GetRelativePath().c_str());
		}
	}

	bool Resource::ResourceManager::Contains(const std::filesystem::path& fullPath)
	{
		return m_resources.contains(fullPath);
	}

	template <typename T>
	inline std::weak_ptr<T> Resource::ResourceManager::GetOrLoad(const std::filesystem::path& fullPath)
	{
		auto relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		auto resource = m_resources.find(relativePath);
		if (resource == m_resources.end())
		{
			// if resource is not imported
			AddResource(new T(fullPath));
			resource = m_resources.find(relativePath);
		}
		if (resource != m_resources.end())
		{
			// Load the resource if not loaded.
			if (!resource->second->p_shouldBeLoaded)
			{
#ifdef ENABLE_MULTITHREAD
				Core::ThreadManager::GetInstance()->AddTask(&IResource::Load, resource->second.get());
#else
				resource->second->Load();
#endif // ENABLE_MULTITHREAD

				return std::dynamic_pointer_cast<T>(resource->second);
			}
			else
			{
				return std::dynamic_pointer_cast<T>(resource->second);
			}
		}

		return std::weak_ptr<T>{};
	}

	template <typename T>
	inline std::weak_ptr<T> Resource::ResourceManager::GetResource(const std::filesystem::path& fullPath)
	{
		auto relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (m_resources.count(relativePath))
		{
			return std::dynamic_pointer_cast<T>(m_resources.at(relativePath));
		}
		return std::weak_ptr<T>{};
	}

	template <typename T>
	inline std::vector<std::weak_ptr<T>> Resource::ResourceManager::GetAllResources()
	{
		std::vector<std::weak_ptr<T>> m_resourcesOfType;
		for (auto&& resource : m_resources)
		{
			if (resource.second->GetFileInfo().GetResourceType() == T::GetResourceType())
			{
				m_resourcesOfType.push_back(std::dynamic_pointer_cast<T>(resource.second));
			}
		}
		return m_resourcesOfType;
	}

	template <typename T>
	[[nodiscard]] inline std::weak_ptr<T> Resource::ResourceManager::ResourcePopup(const char* popupName)
	{
		if (ImGui::BeginPopup(popupName))
		{
			static ImGuiTextFilter filter;
			filter.Draw();
			Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x, 0);
			for (const auto& [path, resource] : m_resources)
			{
				if (resource->GetFileInfo().GetResourceType() == T::GetResourceType() && filter.PassFilter(resource->GetName().c_str()))
				{
					if (ImGui::Button(resource->GetName().c_str(), buttonSize))
					{
						ImGui::CloseCurrentPopup();
						return GetOrLoad<T>(path);
					}
				}
			}
			ImGui::EndPopup();
		}
		return std::weak_ptr<T>();
	}
}
