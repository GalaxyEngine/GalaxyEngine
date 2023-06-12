#include "GalaxyAPI.h"
#include "Resource/IResource.h"

#include <unordered_map>


namespace GALAXY::Resource {
	class ResourceManager
	{
	public:
		~ResourceManager();

		static ResourceManager& GetInstance() { return m_instance; }

		inline void AddResource(IResource* resource)
		{
			m_resources[resource->m_relativepath] = std::shared_ptr<IResource>(resource);
		}

		inline std::weak_ptr<IResource> GetOrLoad(const std::string& relativePath)
		{
			return std::weak_ptr<IResource>{};
		}


		[[nodiscard]] inline std::weak_ptr<IResource> GetResource(const std::string& relativePath)
		{
			if (m_resources.count(relativePath))
				return m_resources.at(relativePath);
			return std::weak_ptr<IResource>{};
		}

	private:
		static ResourceManager m_instance;

		std::unordered_map<std::string, std::shared_ptr<IResource>> m_resources;

	};
}
