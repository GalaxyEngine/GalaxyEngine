#include <array>

namespace GALAXY::Utils::Geometry
{
	struct Triangle
	{
		uint32_t vertex[3];
	};

	constexpr double X = 0.525731112119133606;
	constexpr double Z = 0.850650808352039932;
	constexpr double N = 0.0;

	static const std::vector<Vec3d> verticeData =
	{
		{-X, N, Z}, {X, N, Z}, {-X, N, -Z}, {X, N, -Z},
		{N, Z, X}, {N, Z, -X}, {N, -Z, X}, {N, -Z, -X},
		{Z, X, N}, {-Z, X, N}, {Z, -X, N}, {-Z, -X, N}
	};

	static const std::vector<Triangle> triangleData =
	{
		{0, 4, 1}, {0, 9, 4}, {9, 5, 4}, {4, 5, 8}, {4, 8, 1},
		{8, 10, 1}, {8, 3, 10}, {5, 3, 8}, {5, 2, 3}, {2, 7, 3},
		{7, 10, 3}, {7, 6, 10}, {7, 11, 6}, {11, 0, 6}, {0, 1, 6},
		{6, 1, 10}, {9, 0, 11}, {9, 11, 2}, {9, 2, 5}, {7, 2, 11}
	};

	static uint32_t vertex_for_edge(std::map<std::pair<uint32_t, uint32_t>, uint32_t>& lookup,
	                                std::vector<Vec3d>& vertices, uint32_t first, uint32_t second)
	{
		std::pair key = { first, second };
		if (key.first > key.second)
			std::swap(key.first, key.second);

		auto inserted = lookup.insert({ key, static_cast<uint32_t>(vertices.size()) });
		if (inserted.second)
		{
			Vec3d& edge0 = vertices[first];
			Vec3d& edge1 = vertices[second];
			Vec3d point = (edge0 + edge1).GetNormalize();
			vertices.push_back(point);
		}

		return inserted.first->second;
	}

	static std::vector<Triangle> subdivide(std::vector<Vec3d>& vertices,
	                                       std::vector<Triangle> triangles)
	{
		std::map<std::pair<uint32_t, uint32_t>, uint32_t> lookup;
		std::vector<Triangle> result;

		for (const auto& each : triangles)
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
	
	std::vector<Vec4f> GenerateIcoSphere(int subdivisions)
	{
		std::vector<Vec3d> vertices = verticeData;
		std::vector<Triangle> triangles = triangleData;

		for (int i = 1; i < subdivisions; ++i)
		{
			triangles = subdivide(vertices, triangles);
		}

		std::vector<Vec4f> result;
		result.reserve(triangles.size() * 3); // Correction : on réserve assez d'espace

		for (size_t i = 0; i < triangles.size(); i++)
		{
			result.emplace_back(vertices[triangles[i].vertex[0]], 0.0f);
			result.emplace_back(vertices[triangles[i].vertex[2]], 0.0f);
			result.emplace_back(vertices[triangles[i].vertex[1]], 0.0f);
		}

		return result;
	}
}
