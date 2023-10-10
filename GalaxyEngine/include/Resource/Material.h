#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
namespace GALAXY
{
	namespace Wrapper
	{
		class OBJLoader;
	}
	namespace Render { class Framebuffer; }
	namespace Resource {
		class Material : public IResource
		{
		public:
			Material(const std::filesystem::path& fullPath);
			Material& operator=(const Material& other) = default;
			Material(const Material&) = default;
			Material(Material&&) noexcept = default;
			virtual ~Material() {}

			void Load() override;

			void Save();

			void ShowInInspector() override;

			void SendValues(uint64_t id = -1);

			static ResourceType GetResourceType() { return ResourceType::Material; }

			std::weak_ptr<class Shader> GetShader() const { return m_shader; }

			static Weak<Material> Create(const std::filesystem::path& path);

			Vec4f GetAmbient() const { return m_ambient; }
			Vec4f GetDiffuse() const { return m_diffuse; }
			Vec4f GetSpecular() const { return m_specular; }
			std::weak_ptr<class Texture> GetAlbedo() const { return m_albedo; }

			void SetAmbient(Vec4f val) { m_ambient = val; }
			void SetDiffuse(Vec4f val) { m_diffuse = val; }
			void SetSpecular(Vec4f val) { m_specular = val; }
			void SetAlbedo(std::weak_ptr<class Texture> val) { m_albedo = val; }
			void SetShader(std::weak_ptr<class Shader> val) { m_shader = val; }
		private:
			friend Wrapper::OBJLoader;
			friend Render::Framebuffer;

			std::weak_ptr<class Shader> m_shader;

			Vec4f m_ambient;
			Vec4f m_diffuse;
			Vec4f m_specular;

			std::weak_ptr<class Texture> m_albedo;
			std::weak_ptr<class Texture> m_normal;

		};
	}
}