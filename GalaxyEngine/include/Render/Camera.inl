#pragma once
#include "Render/Camera.h"
namespace GALAXY 
{

	inline void Render::Camera::SetSize(const Vec2i& framebufferSize)
	{
		p_framebufferSize = framebufferSize;
		p_aspectRatio = (float)p_framebufferSize.x / (float)p_framebufferSize.y;
	}

	inline Mat4 Render::Camera::GetViewMatrix() const
	{
		Mat4 out = Mat4::CreateTransformMatrix(GetTransform()->GetWorldPosition(), GetTransform()->GetWorldRotation(), Vec3f(1, 1, -1));
		out = out.CreateInverseMatrix();
		return out;
	}

	inline Mat4 Render::Camera::GetProjectionMatrix() const
	{
		float tanHalfFov = std::tan(p_fov * DegToRad * 0.5f);

		Mat4 projectionMatrix = Mat4();
		projectionMatrix[0][0] = 1.0f / (p_aspectRatio * tanHalfFov);
		projectionMatrix[1][1] = 1.0f / tanHalfFov;
		projectionMatrix[2][2] = (p_far + p_near) / (p_far - p_near);
		projectionMatrix[3][2] = 1.0f;
		projectionMatrix[2][3] = -(2.0f * p_far * p_near) / (p_far - p_near);
		projectionMatrix[3][3] = 0.0f;

		return projectionMatrix;
	}

	inline Mat4 Render::Camera::GetViewProjectionMatrix() const
	{
		return GetProjectionMatrix() * GetViewMatrix();
	}

	inline Vec2f Render::Camera::ToViewport(const Vec2f& pos) const
	{
		return { (float)(2.0f * pos.x) / (float)GetScreenResolution().x - 1.0f, (float)1.0f - (2.0f * pos.y) / (float)GetScreenResolution().y };
	}

	inline Vec3f Render::Camera::UnProject(const Vec3f& point) const
	{
		Vec4f mousePosition = Vec4f(ToViewport(point), 1.f, 1.f);
		Mat4 invVP = GetViewProjectionMatrix().CreateInverseMatrix();
		Vec3f position = GetTransform()->GetWorldPosition();
		Vec4f pos = (invVP * mousePosition);
		return position + (invVP * mousePosition) * point.z;
	}
}
