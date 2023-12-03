#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
#include "Model.h"

namespace GALAXY 
{
	namespace Wrapper { class OBJLoader; }
	namespace Component { class Transform; }
	namespace Resource
	{
		struct SubMesh {
			size_t startIndex = -1;
			size_t count = -1;
		};

		class Mesh : public IResource
		{
		public:
			explicit Mesh(const Path& fullPath);
			Mesh& operator=(const Mesh& other) = default;
			Mesh(const Mesh&) = default;
			Mesh(Mesh&&) noexcept = default;
			~Mesh() override {}

			void Load() override;
			void Send() override;

			void Render(const Mat4& modelMatrix, const std::vector<Weak<class Material>>& materials, uint64_t id = -1) const;

			void DrawBoundingBox(const Component::Transform* transform) const;

			static inline ResourceType GetResourceType() { return ResourceType::Mesh; }

			static Path CreateMeshPath(const Path& modelPath, const Path& fileName);
			inline Resource::BoundingBox GetBoundingBox() const { return m_boundingBox; }
		private:
			void ComputeBoundingBox();

		private:
			friend Wrapper::OBJLoader;
			friend class Model;

			BoundingBox m_boundingBox;

			Model* m_model = nullptr;

			Path m_modelPath;

			std::vector<Vec3f> m_positions;
			std::vector<Vec2f> m_textureUVs;
			std::vector<Vec3f> m_normals;
			std::vector<Vec3i> m_indices;
			std::vector<float> m_finalVertices;
			std::vector<SubMesh> m_subMeshes;

			uint32_t m_vertexArrayIndex = -1;
			uint32_t m_vertexBufferIndex = -1;
			uint32_t m_indexBufferIndex = -1;

		};
	}
}
