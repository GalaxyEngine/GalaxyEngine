#include "GalaxyAPI.h"
#include <unordered_map>


namespace GALAXY::Resource {
	class IResource;
	class ResourceManager
	{
	public:
		~ResourceManager();

		static ResourceManager& GetInstance() { return m_instance; }

	private:
		static ResourceManager m_instance;

		std::unordered_map<std::string, std::shared_ptr<IResource>> m_resources;

	};
}
