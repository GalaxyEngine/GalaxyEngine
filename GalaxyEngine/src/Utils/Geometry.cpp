#include "pch.h"
#include "Utils/Geometry.h"

#include <array>

namespace GALAXY::Utils::Geometry
{
	struct Triangle
	{
		uint32_t vertex[3];
	};

	const double X = .525731112119133606f;
	const double Z = .850650808352039932f;
	const double N = 0.f;

	static const std::vector<Vec3d> verticeData =
	{
	  {-X,N,Z}, {X,N,Z}, {-X,N,-Z}, {X,N,-Z},
	  {N,Z,X}, {N,Z,-X}, {N,-Z,X}, {N,-Z,-X},
	  {Z,X,N}, {-Z,X, N}, {Z,-X,N}, {-Z,-X, N}
	};

	static const std::vector<Triangle> triangleData =
	{
	  {0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
	  {8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
	  {7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
	  {6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
	};

	uint32_t vertex_for_edge(std::map<std::pair<uint32_t, uint32_t>, uint32_t>& lookup,
		std::vector<Vec3d>& vertices, uint32_t first, uint32_t second)
	{
		std::map<std::pair<uint32_t, uint32_t>, uint32_t>::key_type key(first, second);
		if (key.first > key.second)
			std::swap(key.first, key.second);

		auto inserted = lookup.insert({ key, vertices.size() });
		if (inserted.second)
		{
			auto& edge0 = vertices[first];
			auto& edge1 = vertices[second];
			auto point = (edge0 + edge1).GetNormalize();
			vertices.push_back(point);
		}

		return inserted.first->second;
	}

	std::vector<Triangle> subdivide(std::vector<Vec3d>& vertices,
		std::vector<Triangle> triangles)
	{
		std::map<std::pair<uint32_t, uint32_t>, uint32_t> lookup;
		std::vector<Triangle> result;

		for (auto&& each : triangles)
		{
			std::array<uint32_t, 3> mid;
			for (int edge = 0; edge < 3; ++edge)
			{
				mid[edge] = vertex_for_edge(lookup, vertices,
					each.vertex[edge], each.vertex[(edge + 1) % 3]);
			}

			result.push_back({ each.vertex[0], mid[0], mid[2] });
			result.push_back({ each.vertex[1], mid[1], mid[0] });
			result.push_back({ each.vertex[2], mid[2], mid[1] });
			result.push_back({ mid[0], mid[1], mid[2] });
		}

		return result;
	}

	std::vector<Vec4f> generateIcoSphere(int subdivisions)
	{
		std::vector<Vec3d> vertices = verticeData;
		std::vector<Triangle> triangles = triangleData;

		for (int i = 1; i < subdivisions; ++i)
		{
			triangles = subdivide(vertices, triangles);
		}

		std::vector<Vec4f> result;
		result.reserve(triangles.size());

		for (size_t i = 0; i < triangles.size(); i++)
		{
			result.push_back(Vec4f(vertices[triangles[i].vertex[0]], 0));
			result.push_back(Vec4f(vertices[triangles[i].vertex[2]], 0));
			result.push_back(Vec4f(vertices[triangles[i].vertex[1]], 0));
		}

		return result;
	}
}
