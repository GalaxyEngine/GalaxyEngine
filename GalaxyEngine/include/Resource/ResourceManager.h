#include "GalaxyAPI.h"
#include "Resource/IResource.h"
#include "Resource/Texture.h"
#include "Core/ThreadManager.h"

#include <unordered_map>
#include <filesystem>


namespace GALAXY::Resource {
	class ResourceManager
	{
	public:
		~ResourceManager();

		static Resource::ResourceManager* GetInstance();

		// Add or Replace the Resource to the resource Manager
		inline void AddResource(IResource* resource)
		{
			m_resources[resource->p_relativepath] = std::shared_ptr<IResource>(resource);
		}

		inline void AddResource(const std::shared_ptr<IResource>& resource)
		{
			m_resources[resource->p_relativepath] = resource;
		}

		// Get and load the resources if not loaded yet, 
		// import the resource if not inside the resource Manager
		template <typename T>
		inline std::weak_ptr<T> GetOrLoad(const std::string& fullPath)
		{
			std::string relativePath = StringToRelativePath(fullPath);
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

		// Get The Resource, return null if the type is wrong
		template <typename T>
		[[nodiscard]] inline std::weak_ptr<T> GetResource(const std::string& fullPath)
		{
			std::string relativePath = StringToRelativePath(fullPath);
			if (m_resources.count(relativePath))
			{
				return std::dynamic_pointer_cast<T>(m_resources.at(relativePath));
			}
			return std::weak_ptr<T>{};
		}

		template <typename T>
		[[nodiscard]] inline std::vector<std::weak_ptr<T>> GetAllResources()
		{
			std::vector<std::weak_ptr<T>> m_resourcesOfType;
			for (auto&& resource : m_resources)
			{
				if (resource.second->GetType() == T::GetResourceType())
				{
					m_resourcesOfType.push_back(std::dynamic_pointer_cast<T>(resource.second));
				}
			}
			return m_resourcesOfType;
		}

		static std::string StringToRelativePath(const std::string& value);
		static std::string StringToPath(const std::string& value);

		std::weak_ptr<Resource::Shader> GetDefaultShader();

		void ImportAllFilesInFolder(const std::filesystem::path& folder);
		void ImportResource(const std::string& resourcePath);

	private:
		static std::unique_ptr<Resource::ResourceManager> m_instance;

		std::unordered_map<std::string, std::shared_ptr<IResource>> m_resources;

	};
}
