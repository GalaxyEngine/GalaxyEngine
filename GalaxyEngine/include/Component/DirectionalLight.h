#pragma once
#include "GalaxyAPI.h"
#include "Light.h"

namespace GALAXY
{
	namespace Component
	{
		class DirectionalLight : public Light
		{
			COMPONENT_SUBCLASS(DirectionalLight, Light)
		public:
			DirectionalLight() = default;
			DirectionalLight& operator=(const DirectionalLight& other) = default;
			DirectionalLight(const DirectionalLight&) = default;
			DirectionalLight(DirectionalLight&&) noexcept = default;
			~DirectionalLight() override = default;

			void SendLightValues(Resource::Shader* shader) override;

#ifdef WITH_EDITOR
			EDITOR_ONLY void ShowInInspector() override;

			EDITOR_ONLY void OnEditorDraw() override;
#endif
			void ComputeLocationName() override;

			inline Type GetLightType() override { return Light::Type::Directional; }

			inline Vec3f GetDirection() const { return m_direction.value; }
			inline void SetDirection(const Vec3f val) { m_direction.value = val; }

		private:
			LightData<Vec3f> m_direction = Vec3f::Down();
		};
	}
}
