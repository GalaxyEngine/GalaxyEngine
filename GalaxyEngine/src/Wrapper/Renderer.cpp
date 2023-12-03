#include "pch.h"

#include "Wrapper/Renderer.h"
#include "Wrapper/OpenGLRenderer.h"
#include "Wrapper/Window.h"

#include "Core/SceneHolder.h"

#include "Component/Transform.h"

#include "Resource/ResourceManager.h"

namespace GALAXY {
	std::unique_ptr<Wrapper::Renderer> Wrapper::Renderer::m_instance = nullptr;

	void Wrapper::Renderer::CreateInstance(const RenderAPI renderAPI)
	{
		switch (renderAPI)
		{
		case RenderAPI::OPENGL:
		{
			m_instance = std::make_unique<Wrapper::OpenGLRenderer>();
			break;
		}
		case RenderAPI::VULKAN:
			break;
		case RenderAPI::DIRECTX:
			break;
		default:
			break;
		}
		m_instance->Initialize();
		m_instance->EnableDebugOutput();
	}

	void Wrapper::Renderer::DrawWireCube(const Vec3f& pos, const Vec3f& size, const Vec4f& color /*= Vec4f(1)*/, const float lineWidth /*= 1.f*/)
	{
		// Define the eight vertices of the cube
		Vec3f vertices[8];
		vertices[0] = pos + Vec3f(-size.x, -size.y, -size.z);
		vertices[1] = pos + Vec3f(size.x, -size.y, -size.z);
		vertices[2] = pos + Vec3f(size.x, size.y, -size.z);
		vertices[3] = pos + Vec3f(-size.x, size.y, -size.z);
		vertices[4] = pos + Vec3f(-size.x, -size.y, size.z);
		vertices[5] = pos + Vec3f(size.x, -size.y, size.z);
		vertices[6] = pos + Vec3f(size.x, size.y, size.z);
		vertices[7] = pos + Vec3f(-size.x, size.y, size.z);

		// Draw the edges of the cube
		DrawLine(vertices[0], vertices[1], color, lineWidth);
		DrawLine(vertices[1], vertices[2], color, lineWidth);
		DrawLine(vertices[2], vertices[3], color, lineWidth);
		DrawLine(vertices[3], vertices[0], color, lineWidth);
		DrawLine(vertices[4], vertices[5], color, lineWidth);
		DrawLine(vertices[5], vertices[6], color, lineWidth);
		DrawLine(vertices[6], vertices[7], color, lineWidth);
		DrawLine(vertices[7], vertices[4], color, lineWidth);
		DrawLine(vertices[0], vertices[4], color, lineWidth);
		DrawLine(vertices[1], vertices[5], color, lineWidth);
		DrawLine(vertices[2], vertices[6], color, lineWidth);
		DrawLine(vertices[3], vertices[7], color, lineWidth);
	}

	void Wrapper::Renderer::DrawWireCube(Component::Transform* transform, const Vec4f& color /*= Vec4f(1)*/, float lineWidth /*= 1.f*/)
	{
		// Assuming you have a Vec3f representing the center of the cube
		const Vec3f cubeCenter = transform->GetWorldPosition();

		// Assuming you have a float representing the half-size of the cube
		const float halfSize = transform->GetWorldScale().x / 2.0f;

		// Calculate the vertices of the cube
		Vec3f vertices[8];
		vertices[0] = cubeCenter + Vec3f(-halfSize, -halfSize, -halfSize);
		vertices[1] = cubeCenter + Vec3f(halfSize, -halfSize, -halfSize);
		vertices[2] = cubeCenter + Vec3f(-halfSize, halfSize, -halfSize);
		// ... Repeat for the remaining vertices

		// Draw the lines connecting the vertices to form the wireframe cube
		// Assuming you have an instance of OpenGLRenderer named "glRenderer"
		for (int i = 0; i < 4; ++i)
		{
			const int next = (i + 1) % 2;
			DrawLine(vertices[i], vertices[next], color, lineWidth);
			DrawLine(vertices[i + 4], vertices[next + 4], color, lineWidth);
			DrawLine(vertices[i], vertices[i + 4], color, lineWidth);
		}
	}

	void Wrapper::Renderer::DrawWireCube(const Vec3f& pos, const Vec3f& size, const Quat& rotation, const Vec4f& color /*= Vec4f(1)*/, float lineWidth /*= 1.f*/)
	{
		// Define the eight vertices of the cube
		Vec3f vertices[8];
		vertices[0] = pos + rotation * Vec3f(-size.x, -size.y, -size.z);
		vertices[1] = pos + rotation * Vec3f(size.x, -size.y, -size.z);
		vertices[2] = pos + rotation * Vec3f(size.x, size.y, -size.z);
		vertices[3] = pos + rotation * Vec3f(-size.x, size.y, -size.z);
		vertices[4] = pos + rotation * Vec3f(-size.x, -size.y, size.z);
		vertices[5] = pos + rotation * Vec3f(size.x, -size.y, size.z);
		vertices[6] = pos + rotation * Vec3f(size.x, size.y, size.z);
		vertices[7] = pos + rotation * Vec3f(-size.x, size.y, size.z);

		// Draw the edges of the cube
		DrawLine(vertices[0], vertices[1], color, lineWidth);
		DrawLine(vertices[1], vertices[2], color, lineWidth);
		DrawLine(vertices[2], vertices[3], color, lineWidth);
		DrawLine(vertices[3], vertices[0], color, lineWidth);
		DrawLine(vertices[4], vertices[5], color, lineWidth);
		DrawLine(vertices[5], vertices[6], color, lineWidth);
		DrawLine(vertices[6], vertices[7], color, lineWidth);
		DrawLine(vertices[7], vertices[4], color, lineWidth);
		DrawLine(vertices[0], vertices[4], color, lineWidth);
		DrawLine(vertices[1], vertices[5], color, lineWidth);
		DrawLine(vertices[2], vertices[6], color, lineWidth);
		DrawLine(vertices[3], vertices[7], color, lineWidth);
	}

	void Wrapper::Renderer::DrawWireCircle(const Vec3f& pos, const Vec3f& normal, const float radius, const int numSegments /*= 32*/, const Vec4f color /*= Vec4f(1)*/, const float lineWidth /*= 1.f*/)
	{
		float angle = 0.f;
		Vec3f right = Vec3f::Zero();
		if (normal == Vec3f::Up() || normal == -Vec3f::Up())
		{
			right = Vec3f::Right(); // If direction is up or down, use right as the perpendicular vector
		}
		else
		{
			right = Vec3f::Up().Cross(normal).GetNormalize(); // vector to the right of the direction
		}
		const Vec3f startPoint = pos + right * radius;
		Vec3f previousPoint = startPoint;

		for (int i = 1; i <= numSegments; i++)
		{
			angle = i * 360.f / numSegments;
			const Vec3f point = pos + Quat::AngleAxis(angle, normal) * (right * radius);
			DrawLine(previousPoint, point, color, lineWidth);
			previousPoint = point;
		}
		DrawLine(previousPoint, startPoint, color, lineWidth);
	}

	void Wrapper::Renderer::DrawWireCone(const Vec3f& pos, const Quat& rotation, const float topRadius, float angle, float height /*= 25.f*/, const Vec4f& color /*= Vec4f(1)*/, const float lineWidth /*= 1.f*/)
	{
		const Vec3f forward = rotation * Math::Vec3f::Forward();
		constexpr float h = 25.f;
		angle = angle * DegToRad;
		const float radius = topRadius + h * std::tan(angle);
		DrawWireCircle(pos, forward, topRadius, 32, color, lineWidth);
		DrawWireCircle(pos + forward * h, forward, radius, 32, color, lineWidth);
		DrawLine(pos + rotation * Vec3f::Up() * topRadius, pos + rotation * (Vec3f::Up() * radius + Vec3f::Forward() * h), color, lineWidth);
		DrawLine(pos + rotation * -Vec3f::Up() * topRadius, pos + rotation * (-Vec3f::Up() * radius + Vec3f::Forward() * h), color, lineWidth);
		DrawLine(pos + rotation * -Vec3f::Right() * topRadius, pos + rotation * (-Vec3f::Right() * radius + Vec3f::Forward() * h), color, lineWidth);
		DrawLine(pos + rotation * Vec3f::Right() * topRadius, pos + rotation * (Vec3f::Right() * radius + Vec3f::Forward() * h), color, lineWidth);
	}
}