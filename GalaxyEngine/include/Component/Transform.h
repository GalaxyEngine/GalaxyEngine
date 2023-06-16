#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"
#include "Maths/Maths.h"

namespace GALAXY::Component {
	class Transform : public IComponent<Transform>
	{
	public:
		Transform();
		Transform(const Vec3f& position, const Quat& rotation, const Vec3f& scale = { 1, 1, 1 });
		Transform& operator=(const Transform& other) = default;
		Transform(const Transform&) = default;
		Transform(Transform&&) noexcept = default;
		virtual ~Transform() {}

		std::string GetComponentName() const override { return "Transform"; }

		void ShowInInspector() override;

		// === Setters === //
		void SetWorldPosition(const Vec3f& worldPosition);
		void SetWorldRotation(const Quat& worldRotation);
		void SetWorldScale(const Vec3f& worldScale);

		void SetLocalPosition(const Vec3f& localPosition);
		void SetLocalRotation(const Quat& localRotation);
		void SetLocalScale(const Vec3f& localScale);

		// === Getters === //
		Vec3f GetWorldPosition() const;
		Quat  GetWorldRotation() const;
		Vec3f GetWorldScale() const;
		Mat4  GetModelMatrix() const;

		Vec3f GetLocalPosition() const;
		Quat  GetLocalRotation() const;
		Vec3f GetLocalScale() const;
		Mat4  GetLocalMatrix() const;

	private:
		Math::Mat4     m_modelMatrix = Math::Mat4(1);
		Math::Vec3f    m_localPosition = Math::Vec3f();
		Math::Quat	   m_localRotation = Math::Quat();
		Math::Vec3f    m_localEulerRotation = Math::Vec3f();
		Math::Vec3f    m_localScale = Math::Vec3f(1);
		bool m_dirty = true;

		REFLECTION_FRIEND
	};
}