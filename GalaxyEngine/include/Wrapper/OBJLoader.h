#pragma once
#include "GalaxyAPI.h"
#include <vector>
namespace GALAXY
{
	namespace Resource
	{
		class Model;
	}
	namespace Wrapper
	{
		class OBJLoader
		{
		public:
			static void Load(const std::filesystem::path& fullPath, Resource::Model* outputModel);
		private:
			struct OBJSubMesh
			{
				size_t startIndex = -1;
				size_t count = -1;
			};

			struct OBJMesh
			{
				std::filesystem::path name;
				std::vector<OBJSubMesh> subMeshes;
				std::vector<Vec3f> positions;
				std::vector<Vec2f> textureUVs;
				std::vector<Vec3f> normals;
				std::vector<Vec3f> tangents;
				std::vector<Vec3i> indices;
				std::vector<float> finalVertices;
			};

			std::vector<OBJMesh> m_meshes;

			std::filesystem::path m_path;
		private:
			bool Parse();

			static Vec3i ParseFaceIndex(const std::string& indexStr);

			static void ComputeVertices(OBJMesh& mesh);

			static void ConvertQuadToTriangles(const std::vector<Vec3i>& quadIndices, std::vector<Vec3i>& triangleIndices);

			static bool ReadMtl(const std::filesystem::path& mtlPath);
		};
	}
}