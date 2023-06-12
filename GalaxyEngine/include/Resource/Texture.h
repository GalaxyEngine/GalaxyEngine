#include "GalaxyAPI.h"
#include "Resource/IResource.h"
namespace GALAXY::Resource {
	class Texture : public IResource
	{
	public:
		Texture(const std::string& fullPath) : IResource(fullPath) {}
		Texture& operator=(const Texture& other) = default;
		Texture(const Texture&) = default;
		Texture(Texture&&) noexcept = default;
		virtual ~Texture() {}

		void Load() override;
		void Send() override;

	private:
		unsigned char* m_bytes = nullptr;
		Vec2i m_size = {};
	};
}
