#pragma once
#include "GalaxyAPI.h"

#include "Component/PointLight.h"

namespace GALAXY 
{
	namespace Component
	{
		class SpotLight : public PointLight
		{
		public:
			SpotLight() {}
			SpotLight& operator=(const SpotLight& other) = default;
			SpotLight(const SpotLight&) = default;
			SpotLight(SpotLight&&) noexcept = default;
			~SpotLight() override {}

			inline const char* GetComponentName() const override { return "Spot Light"; }

			inline virtual Shared<Component::BaseComponent> Clone() override {
				return std::make_shared<SpotLight>(*dynamic_cast<SpotLight*>(this));
			}

			void OnEditorDraw() override;

			void ShowInInspector() override;

			void ComputeLocationName() override;

			inline Type GetLightType() override { return Light::Type::Spot; };

			void SendLightValues(Resource::Shader* shader) override;

			void Serialize(CppSer::Serializer& serializer) override;
			void Deserialize(CppSer::Parser& parser) override;

			inline void SetCutOff(const float angle) { m_cutOff.value = angle; SetDirty(); }
			inline float GetCutOff() const { return m_cutOff.value; }

			inline void SetOuterCutOff(const float angle) { m_outerCutOff = angle; SetDirty(); }
			inline float GetOuterCutOff() const { return m_outerCutOff.value; }
		private:
			LightData<Vec3f> m_direction;
			LightData<float> m_cutOff = 12.5f;
			LightData<float> m_outerCutOff = 17.5f;
		};
	}
}
