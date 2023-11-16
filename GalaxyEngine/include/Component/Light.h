#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"
namespace GALAXY
{
	namespace Component
	{
		class Light : public IComponent<Light>
		{
		public:
			Light() {}
			Light& operator=(const Light& other) = default;
			Light(const Light&) = default;
			Light(Light&&) noexcept = default;
			virtual ~Light() {}

			inline const char* GetComponentName() const override { return "Light Component"; }

			virtual void OnCreate() override;

			virtual void OnDestroy() override;

			virtual void ShowInInspector() override;

			virtual void Serialize(Utils::Serializer& serializer);
			virtual void Deserialize(Utils::Parser& parser);

			virtual void SendLightValues(Resource::Shader* shader);
			virtual void ResetLightValues(Resource::Shader* shader);

			inline Vec4f GetAmbient() const { return m_ambient; }
			inline void SetAmbient(Vec4f val) { m_ambient = val; }

			inline Vec4f GetDiffuse() const { return m_diffuse; }
			inline void SetDiffuse(Vec4f val) { m_diffuse = val; }

			inline Vec4f GetSpecular() const { return m_specular; }
			inline void SetSpecular(Vec4f val) { m_specular = val; }

		protected:
			Vec4f m_ambient = Vec4f(1);
			Vec4f m_diffuse = Vec4f(1);
			Vec4f m_specular = Vec4f(1);

		};
	}
}
#include "Component/Light.inl" 
