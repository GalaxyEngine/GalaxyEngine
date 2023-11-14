#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
namespace GALAXY
{
	namespace Wrapper
	{
		class MTLLoader;
	}
	namespace Render { class Framebuffer; }
	namespace Resource {
		class Material : public IResource
		{
		public:
			Material(const Path& fullPath);
			Material& operator=(const Material& other) = default;
			Material(const Material&) = default;
			Material(Material&&) noexcept = default;
			virtual ~Material() {}

			void Load() override;

			bool LoadMatFile();

			bool LoadMTLFile();

			void Save();

			void ShowInInspector() override;

			Weak<Shader> SendValues(uint64_t id = -1);

			static inline ResourceType GetResourceType() { return ResourceType::Material; }

			inline Weak<class Shader> GetShader() const { return m_shader; }

			static Weak<Material> Create(const std::filesystem::path& path);

			inline Vec4f GetAmbient() const { return m_ambient; }
			inline Vec4f GetDiffuse() const { return m_diffuse; }
			inline Vec4f GetSpecular() const { return m_specular; }
			inline Weak<class Texture> GetAlbedo() const { return m_albedo; }

			inline void SetAmbient(Vec4f val) { m_ambient = val; }
			inline void SetDiffuse(Vec4f val) { m_diffuse = val; }
			inline void SetSpecular(Vec4f val) { m_specular = val; }
			inline void SetAlbedo(Weak<class Texture> val) { m_albedo = val; }
			inline void SetShader(Weak<class Shader> val) { m_shader = val; }
		private:
			friend Wrapper::MTLLoader;
			friend Render::Framebuffer;

			Weak<class Shader> m_shader;

			Vec4f m_ambient;
			Vec4f m_diffuse;
			Vec4f m_specular;

			Weak<class Texture> m_albedo;
			Weak<class Texture> m_normal;


		};
	}
}