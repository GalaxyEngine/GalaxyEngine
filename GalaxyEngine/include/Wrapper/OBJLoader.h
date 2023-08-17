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
			struct OBJMesh
			{
				std::filesystem::path name;
				std::vector<Vec3f> positions;
				std::vector<Vec2f> textureUVs;
				std::vector<Vec3f> normals;
				std::vector<Vec3i> indices;
				std::vector<float> finalVertices;
			};

			std::vector<OBJMesh> m_meshes;

			std::filesystem::path m_path;
		private:
			bool Parse();

			Vec3i ParseFaceIndex(const std::string& indexStr);

			void ComputeVertices(OBJMesh& mesh);

			void ConvertQuadToTriangles(const std::vector<Vec3i>& quadIndices, std::vector<Vec3i>& triangleIndices);

			bool ReadMtl(const std::filesystem::path& mtlPath);
		};
	}
}