#include "pch.h"
#include "Component/Transform.h"

Component::Transform::Transform()
{

}

Component::Transform::Transform(const Vec3f& position, const Quat& rotation, const Vec3f& scale /*= { 1, 1, 1 }*/)
{

}

void Component::Transform::SetLocalPosition(const Vec3f& localPosition)
{
	m_dirty = true;
	m_localPosition = localPosition;
}

void Component::Transform::SetLocalRotation(const Quat& localRotation)
{
	m_dirty = true;
	m_localRotation = localRotation;
}

void Component::Transform::SetLocalScale(const Vec3f& localScale)
{
	m_dirty = true;
	m_localScale = localScale;
}

void Component::Transform::ShowInInspector()
{
	Vec3f position = m_localPosition;
	Vec3f rotation = m_localEulerRotation;
	Vec3f scale = m_localScale;

	Wrapper::GUI::DrawVec3Control("Position", &position.x);
	Wrapper::GUI::DrawVec3Control("Rotation", &rotation.x);
	Wrapper::GUI::DrawVec3Control("Scale", &scale.x, 1.f, true);

	if (position != m_localPosition || rotation != m_localEulerRotation || scale != m_localScale) {
		SetLocalPosition(position);
		SetLocalRotation(rotation);
		SetLocalScale(scale);
	}
}
