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
		// return Mat4::CreateOrthographicMatrix(-width, width, -height, height, p_near, p_far);
		return Mat4::CreateOrthographicMatrix(-10.f, 10.f, -10.f, 10.f, p_near, p_far);
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

	inline std::array<Vec3f, 8> Render::Camera::GetFrustumCorners() const
	{
		std::array<Vec3f, 8> points;
		Vec2f screenResolution = GetScreenResolution();
		// Calculate all points
		points[0] = UnProject(Vec3f{ 0, 0, p_near });
		points[1] = UnProject(Vec3f{ screenResolution.x, 0, p_near });
		points[2] = UnProject(Vec3f{ 0, screenResolution.y, p_near });
		points[3] = UnProject(Vec3f{ screenResolution, p_near });
		points[4] = UnProject(Vec3f{ 0, 0, p_far });
		points[5] = UnProject(Vec3f{ screenResolution.x, 0, p_far });
		points[6] = UnProject(Vec3f{ 0, screenResolution.y, p_far });
		points[7] = UnProject(Vec3f{ screenResolution, p_far });

		return points;
	}
}
