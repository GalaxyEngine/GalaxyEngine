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
		if (p_viewMode == ViewMode::Orthographic)
			return GetOrthographicMatrix();
		return Mat4::CreateProjectionMatrix(p_fov, p_aspectRatio, p_near, p_far);
	}

	inline Mat4 Render::Camera::GetOrthographicMatrix() const
	{
		const float width = static_cast<float>(p_framebufferSize.x) / 2.f;
		const float height = static_cast<float>(p_framebufferSize.y) / 2.f;

		Mat4 orthographicMatrix = Mat4();
		orthographicMatrix[0][0] = 2.0f / (width - -width);
		orthographicMatrix[1][1] = 2.0f / (height - -height);
		orthographicMatrix[2][2] = -2.0f / (p_far - p_near);
		orthographicMatrix[3][0] = -(width + -width) / (width - -width);
		orthographicMatrix[3][1] = -(height + -height) / (height - -height);
		orthographicMatrix[3][2] = -(p_far + p_near) / (p_far - p_near);
		orthographicMatrix[3][3] = 1.0f;
		return orthographicMatrix;
	}

	inline Mat4 Render::Camera::GetViewProjectionMatrix() const
	{
		return GetProjectionMatrix() * GetViewMatrix();
	}

	inline Vec2f Render::Camera::ToViewport(const Vec2f& pos) const
	{
		return { (2.0f * pos.x) / static_cast<float>(GetScreenResolution().x) - 1.0f, 1.0f - (2.0f * pos.y) / static_cast<float>(GetScreenResolution().y) };
	}

	inline Vec3f Render::Camera::UnProject(const Vec3f& point) const
	{
		const Vec4f mousePosition = Vec4f(ToViewport(point), 1.f, 1.f);
		const Mat4 invVP = GetViewProjectionMatrix().CreateInverseMatrix();
		const Vec3f position = GetTransform()->GetWorldPosition();
		return position + (invVP * mousePosition) * point.z;
	}
}
