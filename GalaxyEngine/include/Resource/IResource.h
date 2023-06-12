#include "GalaxyAPI.h"
#include <string>
namespace GALAXY::Resource {
	enum class ResourceType
	{
		None,
		Texture,
		Shader,
		Model,
		Mesh,
	};

	class GALAXY_API IResource
	{
	public:
		IResource(const std::string& fullPath);
		IResource& operator=(const IResource& other) = default;
		IResource(const IResource&) = default;
		IResource(IResource&&) noexcept = default;
		virtual ~IResource() {}

		static ResourceType GetTypeFromExtension(const std::string_view& ext);
		static std::string ExtractNameFromPath(std::string path, bool extension = true);
		static std::string ExtractRelativePathFromPath(const std::string& path);
		static std::string ExtractExtensionFromPath(const std::string& path);

		virtual void Load() {}
		virtual void Send() {}

	private:
		friend class ResourceManager;

		ResourceType m_type;
		std::string m_fullPath;
		std::string m_relativepath;
		std::string m_name;

		std::atomic_bool m_loaded = false;
		std::atomic_bool m_shouldBeLoaded = false;
		std::atomic_bool m_hasBeenSent = false;

	};
}
