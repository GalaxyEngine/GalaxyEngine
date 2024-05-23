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
			Path GetThumbnailPath() const override;

			bool LoadMatFile();

			static bool LoadMTLFile();

			void Save();

			void ShowInInspector() override;

			void SendForDefault(Shared<Resource::Shader> shader) const;

			Weak<Shader> SendValues(uint64_t id = -1) const;

			static inline ResourceType GetResourceType() { return ResourceType::Material; }

			static Weak<Material> Create(const std::filesystem::path& path);

			inline Shared<Shader> GetShader() const { return m_shader.lock(); }
			inline Vec4f GetAmbient() const { return GetColor("ambient"); }
			inline Vec4f GetDiffuse() const { return GetColor("diffuse"); }
			inline Vec4f GetSpecular() const { return GetColor("specular"); }
			inline Weak<Texture> GetAlbedo() const { return GetTexture("albedo"); }
			inline Weak<Texture> GetNormalMap() const { return GetTexture("normalMap"); }
			inline Weak<Texture> GetParallaxMap() const { return GetTexture("parallaxMap"); }
			inline float GetHeightScale() const { return GetFloat("heightScale"); }

			void SetShader(const Weak<Shader>& val);

			inline bool IsShaderValid() const { return m_shader.lock() && m_shader.lock()->HasBeenSent(); }

			inline void SetBool(const std::string& name, const bool val)
			{
				// Check if shader is valid, if yes check also if the name is in the map else create it because we don't know if we will need it
				if (!IsShaderValid() || IsShaderValid() && m_bools.contains(name))
					m_bools[name] = val;
			}
			inline void SetInteger(const std::string& name, const int val)
			{
				if (!IsShaderValid() || IsShaderValid() && m_ints.contains(name))
					m_ints[name] = val;
			}
			inline void SetFloat(const std::string& name, const float val)
			{
				if (!IsShaderValid() || IsShaderValid() && m_floats.contains(name))
					m_floats[name] = val;
			}
			inline void SetColor(const std::string& name, const Vec4f& val)
			{
				if (!IsShaderValid() || IsShaderValid() && m_float4.contains(name))
					m_float4[name] = val;
			}
			inline void SetTexture(const std::string& name, const Weak<Texture>& val)
			{
				if (!IsShaderValid() || IsShaderValid() && m_textures.contains(name))
					m_textures[name] = val;
			}

			inline bool GetBool(const std::string& name) const { return m_bools.contains(name) ? m_bools.at(name) : false; }
			inline int GetInteger(const std::string& name) const { return m_ints.contains(name) ? m_ints.at(name) : -1; }
			inline float GetFloat(const std::string& name) const { return m_floats.contains(name) ? m_floats.at(name) : 0.0f; }
			inline Vec4f GetColor(const std::string& name) const { return m_float4.contains(name) ? m_float4.at(name) : Vec4f(0.0f, 0.0f, 0.0f, 0.0f); }
			inline Weak<Texture> GetTexture(const std::string& name) const { return m_textures.contains(name) ? m_textures.at(name) : Weak<Texture>(); }
			
			inline void SetAmbient(const Vec4f& val) { SetColor("ambient", val); }
			inline void SetDiffuse(const Vec4f& val) { SetColor("diffuse", val); }
			inline void SetSpecular(const Vec4f& val) { SetColor("specular", val); }
			inline void SetAlbedo(const Weak<Texture>& val)
			{
				SetBool("hasAlbedo", val.lock() ? 1 : 0);
				SetTexture("albedo", val);
			}
			inline void SetNormalMap(const Weak<Texture>& val)
			{
				SetBool("hasNormalMap", val.lock() ? 1 : 0);
				SetTexture("normalMap", val);
			}
			inline void SetParallaxMap(const Weak<Texture>& val) {
				SetBool("hasParallaxMap", val.lock() ? 1 : 0);
				SetTexture("parallaxMap", val);
			}
			inline void SetHeightScale(const float val) { SetFloat("heightScale", val); }

#ifdef WITH_EDITOR
			void CreateThumbnail();
#endif
		private:
			friend Wrapper::MTLLoader;
			friend Wrapper::FBXLoader;
			friend Render::Framebuffer;

			Weak<Shader> m_shader;

			UMap<std::string, bool> m_bools;
			UMap<std::string, float> m_floats;
			UMap<std::string, int> m_ints;
			UMap<std::string, Vec4f> m_float4;
			UMap<std::string, Weak<Texture>> m_textures;

		};
	}
}
