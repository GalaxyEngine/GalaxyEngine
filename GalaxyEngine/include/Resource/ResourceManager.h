#include "GalaxyAPI.h"
#include "Resource/IResource.h"
#include "Resource/Texture.h"

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

		// Get and load the resources if not loaded yet, 
		// import the resource if not inside the resource Manager
		template <typename T>
		inline std::weak_ptr<T> GetOrLoad(const std::string& relativePath)
		{
			std::string path = StringToPath(relativePath);
			auto resource = m_resources.find(path);
			if (resource == m_resources.end())
			{
				// if resource is not imported
				ImportResource(path);
				resource = m_resources.find(path);
			}
			if (resource != m_resources.end())
			{
				// Load the resource if not loaded.
				if (!resource->second->p_shouldBeLoaded) {
					resource->second->Load();
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
		[[nodiscard]] inline std::weak_ptr<T> GetResource(const std::string& relativePath)
		{
			std::string path = StringToPath(relativePath);
			if (m_resources.count(path))
			{
				return std::dynamic_pointer_cast<T>(m_resources.at(path));
			}
			return std::weak_ptr<T>{};
		}

		static std::string StringToPath(const std::string& value);

		void ImportAllFilesInFolder(const std::filesystem::path& folder);
		void ImportResource(const std::string& resourcePath);

	private:
		static std::unique_ptr<Resource::ResourceManager> m_instance;

		std::unordered_map<std::string, std::shared_ptr<IResource>> m_resources;

	};
}
