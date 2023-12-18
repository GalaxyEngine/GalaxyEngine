#pragma once
#include "Component/Transform.h"
namespace GALAXY 
{
	inline void Component::Transform::ComputeModelMatrix(const Mat4& parentMatrix)
	{
		m_modelMatrix = parentMatrix * GetLocalMatrix();
		m_dirty = false;
	}

	inline void Component::Transform::ComputeModelMatrix()
	{
		m_modelMatrix = GetLocalMatrix();
		m_dirty = false;
	}

	inline void Component::Transform::SetLocalPosition(const Vec3f& localPosition)
	{
		m_dirty = true;
		m_localPosition = localPosition;
	}

	inline void Component::Transform::SetLocalRotation(const Quat& localRotation)
	{
		m_dirty = true;
		m_localRotation = localRotation;
		m_localEulerRotation = localRotation.ToEuler();
	}

	inline void Component::Transform::SetLocalRotation(const Vec3f& localRotation)
	{
		m_dirty = true;
		m_localEulerRotation = localRotation;
		m_localRotation = localRotation.ToQuaternion();
	}

	inline void Component::Transform::SetLocalScale(const Vec3f& localScale)
	{
		m_dirty = true;
		m_localScale = localScale;
	}

	inline Vec3f Component::Transform::GetLocalPosition() const
	{
		return m_localPosition;
	}

	inline Quat Component::Transform::GetLocalRotation() const
	{
		return m_localRotation;
	}

	inline Vec3f Component::Transform::GetLocalEulerRotation() const
	{
		return m_localEulerRotation;
	}

	inline Vec3f Component::Transform::GetLocalScale() const
	{
		return m_localScale;
	}

	inline Mat4 Component::Transform::GetLocalMatrix() const
	{
		return Mat4::CreateTransformMatrix(m_localPosition, m_localRotation, m_localScale);
	}

	inline const Mat4& Component::Transform::GetModelMatrix() const
	{
		return m_modelMatrix;
	}

	inline Vec3f Component::Transform::GetUp() const
	{
		return GetWorldRotation() * Vec3f::Up();
	}

	inline Vec3f Component::Transform::GetRight() const
	{
		return GetWorldRotation() * Vec3f::Right();
	}

	inline Vec3f Component::Transform::GetForward() const
	{
		return GetWorldRotation() * Vec3f::Forward();
	}

	inline void Component::Transform::Rotate(const Vec3f axis, const float angle, const Space relativeTo /*= Space::Local*/)
	{
		if (relativeTo == Space::Local)
			RotateAround(TransformDirection(axis), angle);
		else
			RotateAround(axis, angle);
	}

	inline void Component::Transform::RotateAround(const Vec3f point, const Vec3f axis, const float angle)
	{
		const Quat q = Quat::AngleAxis(angle, axis);
		Vec3f dif = GetWorldPosition() - point;
		dif = q * dif;
		SetWorldPosition(point + dif);
		const Quat worldRotation = GetWorldRotation();
		SetWorldRotation(worldRotation * worldRotation.GetInverse() * q * worldRotation);
	}

	inline void Component::Transform::RotateAround(const Vec3f axis, const float angle)
	{
		if (angle == 0.0f)
			return;
		const Quat rotation = Quat::AngleAxis(angle, axis);

		const Vec3f pivot = GetWorldPosition();
		Vec3f relativePosition = Vec3f::Zero();

		// Rotate the relative position
		relativePosition = rotation * relativePosition;

		// Update the object's position
		SetWorldPosition(pivot + relativePosition);

		// Update the object's rotation
		const Quat worldRotation = GetWorldRotation();
		const Quat rot = worldRotation * worldRotation.GetInverse() * rotation * worldRotation;
		SetWorldRotation(rot);
	}

	inline Vec3f Component::Transform::TransformDirection(const Vec3f direction) const
	{
		return GetWorldRotation() * direction;
	}
}
