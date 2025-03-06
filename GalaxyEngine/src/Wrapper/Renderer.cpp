#include "pch.h"

#include "Wrapper/Renderer.h"
#include "Wrapper/RendererAPI/OpenGLRenderer.h"
#include "Wrapper/Window.h"

#include "Core/SceneHolder.h"
#include "Utils/Geometry.h"

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
			m_instance = std::make_unique<Wrapper::RendererAPI::OpenGLRenderer>();
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

	void Wrapper::Renderer::RenderDebug()
	{
		for (auto& line : p_debugLines)
		{
			Internal_DrawLine(line.posA, line.posB, line.color, line.lineWidth);
		}
		p_debugLines.clear();

		if (p_debugTriangles.size())
			Internal_DrawTriangles(p_debugTriangles);
		p_debugTriangles.clear();
	}

	UMap<std::string, Resource::Uniform> Wrapper::Renderer::GetShaderUniforms(Resource::Shader* shader)
	{
		return {};
	}

	void Wrapper::Renderer::AddDebugLine(const DebugLine& line)
	{
		p_debugLines.push_back(line);
	}

	void Wrapper::Renderer::DrawLine(Vec3f pos1, Vec3f pos2, Vec4f color, float lineWidth)
	{
		if (p_renderType == Render::RenderType::None)
		{
			DebugLine debugLine;
			debugLine.posA = pos1;
			debugLine.posB = pos2;
			debugLine.color = color;
			debugLine.lineWidth = lineWidth;
			AddDebugLine(debugLine);
		}
		else
		{
			m_instance->Internal_DrawLine(pos1, pos2, color, lineWidth);
		}
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

	void Wrapper::Renderer::DrawSimpleWireSphere(const Vec3f& pos, float radius, int numSegments, Vec4f color,float lineWidth)
	{
		DrawWireCircle(pos, Vec3f::Up(), radius, numSegments, color, lineWidth);
		DrawWireCircle(pos, Vec3f::Right(), radius, numSegments, color, lineWidth);
		DrawWireCircle(pos, Vec3f::Forward(), radius, numSegments, color, lineWidth);
	}

	void Wrapper::Renderer::DrawWireSphere(const Vec3f& pos, float radius, int numSegments, int numRings, Vec4f color, float lineWidth)
	{
		const float degToRad = DegToRad;  
		const float invNumRings = 1.0f / numRings;  
		const Vec3f up = Vec3f::Up();  

		// Draw longitudinal rings (vertical rings)
		const float longStep = 360.f * invNumRings;  
		for (int i = 0; i < numRings; ++i)
		{
			float angle = i * longStep;
			Vec3f direction = Quat::AngleAxis(angle, up) * Vec3f::Forward();
			DrawWireCircle(pos, direction, radius, numSegments, color, lineWidth);
		}

		// Draw latitude rings (horizontal rings)
		const float latStep = 180.f * invNumRings;  
		for (int i = 0; i <= numRings; ++i)
		{
			float theta = -90.f + (i * latStep);
			float radTheta = theta * degToRad;

			float circleRadius = radius * cos(radTheta);
			float yOffset = radius * sin(radTheta);
			Vec3f circleCenter = pos + Vec3f(0, yOffset, 0);

			DrawWireCircle(circleCenter, up, circleRadius, numSegments, color, lineWidth);
		}
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

	void Wrapper::Renderer::DrawTriangle(Vec3f pos1, Vec3f pos2, Vec3f pos3, Vec4f color)
	{
		p_debugTriangles.push_back(pos1);
		p_debugTriangles.push_back(color);
		p_debugTriangles.push_back(pos2);
		p_debugTriangles.push_back(color);
		p_debugTriangles.push_back(pos3);
		p_debugTriangles.push_back(color);
	}

	void Wrapper::Renderer::DrawCube(const Vec3f& pos, const Vec3f& size, const Quat& rotation /*= Quat::Identity()*/, const Vec4f& color /*= Vec4f(1)*/)
	{
		Vec3f vertices[8];
		vertices[0] = pos + rotation * Vec3f(-size.x, -size.y, -size.z);
		vertices[1] = pos + rotation * Vec3f(size.x, -size.y, -size.z);
		vertices[2] = pos + rotation * Vec3f(-size.x, size.y, -size.z);
		vertices[3] = pos + rotation * Vec3f(size.x, size.y, -size.z);
		vertices[4] = pos + rotation * Vec3f(-size.x, -size.y, size.z);
		vertices[5] = pos + rotation * Vec3f(size.x, -size.y, size.z);
		vertices[6] = pos + rotation * Vec3f(-size.x, size.y, size.z);
		vertices[7] = pos + rotation * Vec3f(size.x, size.y, size.z);

		DrawTriangle(vertices[0], vertices[2], vertices[1], color);
		DrawTriangle(vertices[1], vertices[2], vertices[3], color);
		DrawTriangle(vertices[4], vertices[5], vertices[6], color);
		DrawTriangle(vertices[5], vertices[7], vertices[6], color);

		DrawTriangle(vertices[1], vertices[7], vertices[5], color);
		DrawTriangle(vertices[1], vertices[3], vertices[7], color);
		DrawTriangle(vertices[0], vertices[4], vertices[6], color);
		DrawTriangle(vertices[0], vertices[6], vertices[2], color);

		DrawTriangle(vertices[2], vertices[7], vertices[3], color);
		DrawTriangle(vertices[2], vertices[6], vertices[7], color);
		DrawTriangle(vertices[0], vertices[1], vertices[5], color);
		DrawTriangle(vertices[0], vertices[5], vertices[4], color);
	}

	void Wrapper::Renderer::DrawCircle(const Vec3f& pos, const Vec3f& normal, const float radius, const int numSegments /*= 32*/, const Vec4f color /*= Vec4f(1)*/)
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
			DrawTriangle(pos, previousPoint, point, color);
			previousPoint = point;
		}
		DrawTriangle(pos, previousPoint, startPoint, color);
	}

	void Wrapper::Renderer::DrawSphere(const Vec3f pos, const Quat& rotation, float radius, int subdivisions, Vec4f color)
	{
		static std::vector<std::vector<Vec4f>> meshData = std::vector<std::vector<Vec4f>>(10);
		if (subdivisions > 10)
			subdivisions = 10;
		if (subdivisions < 1)
			subdivisions = 1;


		if (meshData[subdivisions-1].empty())
		{
			meshData[subdivisions-1] = Utils::Geometry::generateIcoSphere(subdivisions);
		}

		auto& sphere = meshData[subdivisions-1];
		for (int i = 0; i < sphere.size() / 3; i++)
		{
			DrawTriangle(pos + radius * (rotation * Vec3f(sphere[i * 3])),
				pos + radius * (rotation * Vec3f(sphere[i * 3 + 1])),
				pos + radius * (rotation * Vec3f(sphere[i * 3 + 2])), color);
		}
	}
}
