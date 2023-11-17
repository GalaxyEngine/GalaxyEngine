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
			enum class Type
			{
				None = -1,
				Directional,
				Point,
				Spot
			};

			template <typename T>
			struct LightData
			{
				std::string string;
				T value;

				LightData() = default;
				LightData(std::string value, T val) : string(std::move(value)), value(val) {}
				LightData(T val) : value(val) {}

				operator T() const { return value; }
			};
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

			virtual void Serialize(Utils::Serializer& serializer) override;
			virtual void Deserialize(Utils::Parser& parser) override;

			virtual void SendLightValues(Resource::Shader* shader);
			virtual void ResetLightValues(Resource::Shader* shader);

			virtual void ComputeLocationName();

			inline Vec4f GetAmbient() const { return p_ambient.value; }
			inline void SetAmbient(Vec4f val) { p_ambient.value = val; SetDirty(); }

			inline Vec4f GetDiffuse() const { return p_diffuse.value; }
			inline void SetDiffuse(Vec4f val) { p_diffuse.value = val; SetDirty(); }

			inline Vec4f GetSpecular() const { return p_specular.value; }
			inline void SetSpecular(Vec4f val) { p_specular.value = val; SetDirty(); }

			inline virtual Type GetLightType() { return Type::None; };

			inline size_t GetIndex() const { return p_index; }
			inline void SetIndex(size_t val) { p_index = val; ComputeLocationName(); }

			inline bool IsDirty() const { return p_dirty; }
			inline void SetDirty() { p_dirty = true; }
		protected:
			size_t p_index = -1;

			LightData<Vec4f> p_ambient = Vec4f(0);
			LightData<Vec4f> p_diffuse = Vec4f(1);
			LightData<Vec4f> p_specular = Vec4f(1);

			std::string p_enableString;

			bool p_dirty = true;
		};
	}
}
#include "Component/Light.inl" 
