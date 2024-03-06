#include "pch.h"
#include "Physic/Plane.h"
namespace GALAXY 
{

	void Physic::Plane::DebugDraw() const
	{
		auto renderer = Wrapper::Renderer::GetInstance();

		// Define the size of the square
		const float size = 10.0f;

		// Calculate two perpendicular vectors on the plane
		Vec3f tangent;

		// If the normal is not parallel to the x-axis, we can cross it with the x-axis.
		if (std::abs(normal.x) > std::numeric_limits<float>::epsilon()) {
			tangent = normal.Cross(Vec3f(0, 1, 0)); // Using the y-axis if the normal has a significant x component
		}
		// If the normal is parallel to the x-axis, we cross it with the y-axis.
		else {
			tangent = normal.Cross(Vec3f(1, 0, 0)); // Using the x-axis otherwise
		}
		Vec3f bitangent = normal.Cross(tangent);
		tangent.Normalize();
		bitangent.Normalize();

		// Scale them to the desired size
		tangent *= size;
		bitangent *= size;

		// Calculate a point on the plane (using the plane equation)
		Vec3f pointOnPlane = normal * -distance;

		// Define the four corners of the square
		Vec3f points[4] = {
			pointOnPlane + tangent - bitangent, // Top-left
			pointOnPlane + tangent + bitangent, // Top-right
			pointOnPlane - tangent + bitangent, // Bottom-right
			pointOnPlane - tangent - bitangent  // Bottom-left
		};

		// Draw the lines between the corners
		renderer->DrawLine(points[0], points[1], Vec4f(1, 0, 0, 1), 5);
		renderer->DrawLine(points[1], points[2], Vec4f(1, 0, 0, 1), 5);
		renderer->DrawLine(points[2], points[3], Vec4f(1, 0, 0, 1), 5);
		renderer->DrawLine(points[3], points[0], Vec4f(1, 0, 0, 1), 5);
	}

}
