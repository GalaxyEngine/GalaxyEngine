#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"
#ifdef WITH_EDITOR
#include "Editor/EditorIcon.h"
#endif
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
			Light() = default;
			Light& operator=(const Light& other) = default;
			Light(const Light&) = default;
			Light(Light&&) noexcept = default;
			~Light() override = default;

			inline const char* GetComponentName() const override { return "Light Component"; }

			virtual void OnCreate() override;

			virtual void OnDestroy() override;

			virtual void OnEditorDraw() override;

			EDITOR_ONLY virtual void ShowInInspector() override;

			virtual void Serialize(CppSer::Serializer& serializer) override;
			virtual void Deserialize(CppSer::Parser& parser) override;

			virtual void SendLightValues(Resource::Shader* shader);
			virtual void ResetLightValues(Resource::Shader* shader);

			virtual void ComputeLocationName();

			inline Vec4f GetAmbient() const { return p_ambient.value; }
			inline void SetAmbient(const Vec4f val) { p_ambient.value = val; SetDirty(); }

			inline Vec4f GetDiffuse() const { return p_diffuse.value; }
			inline void SetDiffuse(const Vec4f val) { p_diffuse.value = val; SetDirty(); }

			inline Vec4f GetSpecular() const { return p_specular.value; }
			inline void SetSpecular(const Vec4f val) { p_specular.value = val; SetDirty(); }

			inline virtual Type GetLightType() { return Type::None; };

			inline size_t GetLightIndex() const { return p_lightIndex; }
			inline void SetLightIndex(const size_t val) { p_lightIndex = val; ComputeLocationName(); }

			inline bool IsDirty() const { return p_dirty; }
			inline void SetDirty() { p_dirty = true; }
		protected:
			size_t p_lightIndex = -1;

			LightData<Vec3f> p_ambient = Vec3f(0);
			LightData<Vec3f> p_diffuse = Vec3f(1);
			LightData<Vec3f> p_specular = Vec3f(1);

			std::string p_enableString;

			bool p_dirty = true;

#ifdef WITH_EDITOR
			Editor::EditorIcon m_editorIcon;
#endif
		};
	}
}
