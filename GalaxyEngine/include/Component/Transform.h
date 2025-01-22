#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"
#include <galaxymath/Maths.h>

#include "Utils/Event.h"

namespace GALAXY {
	enum class Space
	{
		Local,
		World,
	};
	namespace Component {
		class GALAXY_API Transform : public IComponent<Transform>
		{
		public:
			Transform();
			Transform(const Vec3f& position, const Quat& rotation, const Vec3f& scale = Vec3f(1.f));
			Transform& operator=(const Transform& other) = default;
			Transform(const Transform&) = default;
			Transform(Transform&&) noexcept = default;
			~Transform() override = default;

			void OnUpdate() override;
			void ForceUpdate();

			inline void ComputeModelMatrix();
			inline void ComputeModelMatrix(const Mat4& parentMatrix);

			inline const char* GetComponentName() const override { return "Transform"; }

			EDITOR_ONLY void ShowInInspector() override;

			void Serialize(CppSer::Serializer& serializer) override;
			void Deserialize(CppSer::Parser& parser) override;

			// === Setters === //
			void SetWorldPosition(const Vec3f& worldPosition);
			void SetWorldRotation(const Quat& worldRotation);
			void SetWorldRotation(const Vec3f& worldRotation);
			void SetWorldScale(const Vec3f& worldScale);

			inline void SetLocalPosition(const Vec3f& localPosition);
			inline void SetLocalRotation(const Quat& localRotation);
			inline void SetLocalRotation(const Vec3f& localRotation);
			inline void SetLocalScale(const Vec3f& localScale);

			// === Getters === //
			[[nodiscard]] Vec3f GetWorldPosition() const;
			[[nodiscard]] Quat  GetWorldRotation() const;
			[[nodiscard]] Vec3f GetWorldEulerRotation() const;
			[[nodiscard]] Vec3f GetWorldScale() const;
			[[nodiscard]] inline const Mat4& GetModelMatrix() const;

			[[nodiscard]] inline Vec3f GetLocalPosition() const;
			[[nodiscard]] inline Quat  GetLocalRotation() const;
			[[nodiscard]] inline Vec3f GetLocalEulerRotation() const;
			[[nodiscard]] inline Vec3f GetLocalScale() const;
			[[nodiscard]] inline Mat4 GetLocalMatrix() const;

			[[nodiscard]] inline Vec3f GetForward() const;
			[[nodiscard]] inline Vec3f GetRight() const;
			[[nodiscard]] inline Vec3f GetUp() const;

			// === Other Methods === //
			inline Vec3f TransformDirection(Vec3f direction) const;

			inline void RotateAround(Vec3f axis, float angle);

			inline void RotateAround(Vec3f point, Vec3f axis, float angle);

			inline void Rotate(Vec3f axis, float angle, Space relativeTo = Space::Local);

			inline bool WasDirty() const { return m_wasDirty; }
			
			Utils::Event<> EOnUpdate;
		private:
			Math::Mat4     m_modelMatrix = Math::Mat4(1);
			Math::Vec3f    m_localPosition = Math::Vec3f();
			Math::Quat	   m_localRotation = Math::Quat();
			Math::Vec3f    m_localEulerRotation = Math::Vec3f();
			Math::Vec3f    m_localScale = Math::Vec3f(1);

			bool m_dirty = true;
			bool m_wasDirty = false;

			//REFLECTION_FRIEND
		};
	}
}
#include "Component/Transform.inl" 