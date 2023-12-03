#pragma once
#include "GalaxyAPI.h"
#include "Light.h"

namespace GALAXY
{
	namespace Component
	{
		class DirectionalLight : public Light
		{
		public:
			DirectionalLight() = default;
			DirectionalLight& operator=(const DirectionalLight& other) = default;
			DirectionalLight(const DirectionalLight&) = default;
			DirectionalLight(DirectionalLight&&) noexcept = default;
			~DirectionalLight() override = default;

			inline const char* GetComponentName() const override { return "Directional Light"; }

			inline virtual Shared<Component::BaseComponent> Clone() override {
				return std::make_shared<DirectionalLight>(*dynamic_cast<DirectionalLight*>(this));
			}

			void SendLightValues(Resource::Shader* shader) override;

			void ShowInInspector() override;

			void OnEditorDraw() override;

			void ComputeLocationName() override;

			inline Type GetLightType() override { return Light::Type::Directional; }

			inline Vec3f GetDirection() const { return m_direction.value; }
			inline void SetDirection(const Vec3f val) { m_direction.value = val; }

		private:
			LightData<Vec3f> m_direction = Vec3f::Down();
		};
	}
}
