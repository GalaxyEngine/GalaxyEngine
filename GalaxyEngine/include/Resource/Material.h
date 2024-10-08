#pragma once
#include "GalaxyAPI.h"
#include "Shader.h"

namespace GALAXY
{
	namespace Wrapper
	{
		class MTLLoader;
		class FBXLoader;
	}
	namespace Render { class Framebuffer; }
	namespace Resource {
		class Cubemap;
		class Texture;
		struct MaterialData
		{
			UMap<std::string, bool> m_bools;
			UMap<std::string, float> m_floats;
			UMap<std::string, int> m_ints;
			UMap<std::string, Vec4f> m_float4;
			
			UMap<std::string, Weak<Texture>> m_textures;
			UMap<std::string, Weak<Cubemap>> m_cubemaps;
		};
		
		class Texture;
		class Material : public IResource
		{
		public:
			explicit Material(const Path& fullPath);
			Material& operator=(const Material& other) = default;
			Material(const Material&) = default;
			Material(Material&&) noexcept = default;
			~Material() override {}

			void Load() override;

			void OnAdd() override;

			const char* GetResourceName() const override { return "Material"; }
			
#ifdef WITH_EDITOR
			EDITOR_ONLY Path GetThumbnailPath() const override;
#endif

			bool LoadMatFile();

			static bool LoadMTLFile();

			void Save();

			EDITOR_ONLY void ShowInInspector() override;

			void SendForDefault(Shared<Resource::Shader> shader) const;

			Weak<Shader> SendValues(uint64_t id = -1) const;

			static inline ResourceType GetResourceType() { return ResourceType::Material; }

			static Weak<Material> Create(const std::filesystem::path& path);

			Shared<Shader> GetShader() const { return m_shader.lock(); }
			Vec4f GetAmbient() const { return GetColor("ambient"); }
			Vec4f GetDiffuse() const { return GetColor("diffuse"); }
			Vec4f GetSpecular() const { return GetColor("specular"); }
			Weak<Texture> GetAlbedo() const { return GetTexture("albedo"); }
			Weak<Texture> GetNormalMap() const { return GetTexture("normalMap"); }
			Weak<Texture> GetParallaxMap() const { return GetTexture("parallaxMap"); }
			float GetHeightScale() const { return GetFloat("heightScale"); }

			void SetShader(const Weak<Shader>& val);

			bool IsShaderValid() const { return m_shader.lock() && m_shader.lock()->HasBeenSent(); }

			void SetBool(const std::string& name, const bool val);

			void SetInteger(const std::string& name, const int val);

			void SetFloat(const std::string& name, const float val);

			void SetColor(const std::string& name, const Vec4f& val);

			void SetTexture(const std::string& name, const Weak<Texture>& val);

			void SetCubemap(const std::string& name, const Weak<Cubemap>& val);

			bool GetBool(const std::string& name) const;
			const UMap<std::string, bool>& GetBools() const;
			int GetInteger(const std::string& name) const;
			const UMap<std::string, int>& GetIntegers() const;
			float GetFloat(const std::string& name) const;
			const UMap<std::string, float>& GetFloats() const;
			Vec4f GetColor(const std::string& name) const;
			const UMap<std::string, Vec4f>& GetColors() const;
			Weak<Texture> GetTexture(const std::string& name) const;
			const UMap<std::string, Weak<Texture>>& GetTextures() const;
			Weak<Cubemap> GetCubemap(const std::string& name) const;
			const UMap<std::string, Weak<Cubemap>>& GetCubemaps() const;

			void SetAmbient(const Vec4f& val) { SetColor("ambient", val); }
			void SetDiffuse(const Vec4f& val) { SetColor("diffuse", val); }
			void SetSpecular(const Vec4f& val) { SetColor("specular", val); }
			void SetAlbedo(const Weak<Texture>& val)
			{
				SetBool("hasAlbedo", val.lock() ? 1 : 0);
				SetTexture("albedo", val);
			}
			void SetNormalMap(const Weak<Texture>& val)
			{
				SetBool("hasNormalMap", val.lock() ? 1 : 0);
				SetTexture("normalMap", val);
			}
			void SetParallaxMap(const Weak<Texture>& val) {
				SetBool("hasParallaxMap", val.lock() ? 1 : 0);
				SetTexture("parallaxMap", val);
			}
			void SetHeightScale(const float val) { SetFloat("heightScale", val); }

#ifdef WITH_EDITOR
			EDITOR_ONLY void CreateThumbnail();
#endif

			static void OnShaderLoaded(const Weak<IResource>& material, const Weak<Shader>& shader);
		private:
			friend Wrapper::MTLLoader;
			friend Wrapper::FBXLoader;
			friend Render::Framebuffer;

			Weak<Shader> m_shader;

			MaterialData m_data;
			MaterialData m_tempData; // The data is store here when the shader is loading
		};
	}
}
