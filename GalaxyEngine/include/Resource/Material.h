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
			Material(const std::filesystem::path& fullPath);
			Material& operator=(const Material& other) = default;
			Material(const Material&) = default;
			Material(Material&&) noexcept = default;
			virtual ~Material() {}

			void ShowInInspector() override;

			void SendValues();

			static ResourceType GetResourceType() { return ResourceType::Material; }
			std::weak_ptr<class Shader> GetShader() const { return m_shader; }
		private:
			friend Wrapper::OBJLoader;
			std::weak_ptr<class Shader> m_shader;

			Vec4f m_ambient;
			Vec4f m_diffuse;
			Vec4f m_specular;

			std::weak_ptr<class Texture> m_albedo;
			std::weak_ptr<class Texture> m_normal;

		};
	}
}