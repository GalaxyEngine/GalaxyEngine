#pragma once
#include "GalaxyAPI.h"
#include "Component/Light.h"

namespace GALAXY 
{
	namespace Component
	{
		class PointLight : public Light
		{
			COMPONENT_SUBCLASS(PointLight, Light)
		public:
			PointLight() {}
			PointLight& operator=(const PointLight& other) = default;
			PointLight(const PointLight&) = default;
			PointLight(PointLight&&) noexcept = default;
			~PointLight() override {}
			
			inline Type GetLightType() override { return Light::Type::Point; };

			void SendLightValues(Resource::Shader* shader) override;

#ifdef WITH_EDITOR
			EDITOR_ONLY void ShowInInspector() override;
#endif
			
			inline float GetConstant() const { return p_constant.value; }
			inline void SetConstant(const float val) { p_constant.value = val; SetDirty(); }

			inline float GetLinear() const { return p_linear.value; }
			inline void SetLinear(const float val) { p_linear.value = val; SetDirty(); }

			inline float GetQuadratic() const { return p_quadratic.value; }
			inline void SetQuadratic(const float val) { p_quadratic.value = val; SetDirty(); }

			void ComputeLocationName() override;

			void Serialize(CppSer::Serializer& serializer) override;
			void Deserialize(CppSer::Parser& parser) override;
		protected:
			LightData<Vec3f> p_position;
			LightData<float> p_constant = 1.0f;
			LightData<float> p_linear = 0.35f;
			LightData<float> p_quadratic = 0.44f;
		};
	}
}
