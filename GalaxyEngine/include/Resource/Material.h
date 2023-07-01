#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
namespace GALAXY
{
	namespace Wrapper
	{
		class OBJLoader;
	}
	namespace Resource {
		class Material : public IResource
		{
		public:
			Material(const std::string& fullPath) : IResource(fullPath) {}
			Material& operator=(const Material& other) = default;
			Material(const Material&) = default;
			Material(Material&&) noexcept = default;
			virtual ~Material() {}

			void ShowInInspector() override;

			static ResourceType GetResourceType() { return ResourceType::Material; }
		private:
			friend Wrapper::OBJLoader;

			Vec4f m_ambient;
			Vec4f m_diffuse;
			Vec4f m_specular;

			std::weak_ptr<class Texture> m_albedo;
			std::weak_ptr<class Texture> m_normal;

		};
	}
}