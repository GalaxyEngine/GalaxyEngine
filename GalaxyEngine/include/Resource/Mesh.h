#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
namespace GALAXY 
{
	namespace Wrapper { class OBJLoader; }
	namespace Resource
	{
		class Mesh : public IResource
		{
		public:
			Mesh(const Path& fullPath);
			Mesh& operator=(const Mesh& other) = default;
			Mesh(const Mesh&) = default;
			Mesh(Mesh&&) noexcept = default;
			virtual ~Mesh() {}

			void Load() override;
			void Send() override;

			void Render(const Mat4& modelMatrix, const std::vector<std::weak_ptr<class Material>>& materials, uint64_t id = -1);

			static inline ResourceType GetResourceType() { return ResourceType::Mesh; }

			static Path CreateMeshPath(const Path& modelPath, const Path& fileName);

		private:
			friend Wrapper::OBJLoader;
			friend class Model;

			Path m_modelPath;

			std::vector<Vec3f> m_positions;
			std::vector<Vec2f> m_textureUVs;
			std::vector<Vec3f> m_normals;
			std::vector<Vec3i> m_indices;
			std::vector<float> m_finalVertices;

			uint32_t m_vertexArrayIndex = -1;
			uint32_t m_vertexBufferIndex = -1;
			uint32_t m_indexBufferIndex = -1;

		};
	}
}
