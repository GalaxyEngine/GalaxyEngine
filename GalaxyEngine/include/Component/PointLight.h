#pragma once
#include "GalaxyAPI.h"
#include "Component/Light.h"

namespace GALAXY 
{
	namespace Component
	{
		class PointLight : public Light
		{
		public:
			PointLight() {}
			PointLight& operator=(const PointLight& other) = default;
			PointLight(const PointLight&) = default;
			PointLight(PointLight&&) noexcept = default;
			virtual ~PointLight() {}

			inline const char* GetComponentName() const override { return "Point Light"; }

			inline virtual Shared<Component::BaseComponent> Clone() override {
				return std::make_shared<PointLight>(*dynamic_cast<PointLight*>(this));
			}
			
			inline Type GetLightType() override { return Light::Type::Point; };

			void SendLightValues(Resource::Shader* shader) override;

			void ShowInInspector() override;

			inline float GetConstant() const { return m_constant.value; }
			inline void SetConstant(float val) { m_constant.value = val; }

			inline float GetLinear() const { return m_linear.value; }
			inline void SetLinear(float val) { m_linear.value = val; }

			inline float GetQuadratic() const { return m_quadratic.value; }
			inline void SetQuadratic(float val) { m_quadratic.value = val; }

			void SetIndex(size_t val) override;

			void Serialize(Utils::Serializer& serializer) override;
			void Deserialize(Utils::Parser& parser) override;
		private:
			LightData<float> m_constant = 1.0f;
			LightData<float> m_linear = 0.35f;
			LightData<float> m_quadratic = 0.44f;

			std::string m_positionString;
		};
	}
}
