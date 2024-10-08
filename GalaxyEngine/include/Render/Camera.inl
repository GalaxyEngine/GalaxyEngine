#pragma once
#include "Render/Camera.h"
namespace GALAXY 
{

	inline void Render::Camera::SetSize(const Vec2i& framebufferSize)
	{
		p_framebufferSize = framebufferSize;
		p_aspectRatio = static_cast<float>(p_framebufferSize.x) / static_cast<float>(p_framebufferSize.y);
	}

	inline Mat4 Render::Camera::GetViewMatrix() const
	{
		//Mat4 out = Mat4::CreateTransformMatrix(GetTransform()->GetWorldPosition(), GetTransform()->GetWorldRotation(), Vec3f(1, 1, 1));
		//out = out.CreateInverseMatrix();
		return GetTransform()->GetModelMatrix().CreateInverseMatrix();
	}

	inline Mat4 Render::Camera::GetProjectionMatrix() const
	{
		return Mat4::CreateProjectionMatrix(p_fov, p_aspectRatio, p_near, p_far);
	}

	inline Mat4 Render::Camera::GetViewProjectionMatrix() const
	{
		return GetProjectionMatrix() * GetViewMatrix();
	}

	inline Vec2f Render::Camera::ToViewport(const Vec2f& pos) const
	{
		return { (2.0f * pos.x) / static_cast<float>(GetScreenResolution().x) - 1.0f, (float)1.0f - (2.0f * pos.y) / (float)GetScreenResolution().y };
	}

	inline Vec3f Render::Camera::UnProject(const Vec3f& point) const
	{
		const Vec4f mousePosition = Vec4f(ToViewport(point), 1.f, 1.f);
		const Mat4 invVP = GetViewProjectionMatrix().CreateInverseMatrix();
		const Vec3f position = GetTransform()->GetWorldPosition();
		return position + (invVP * mousePosition) * point.z;
	}
}
