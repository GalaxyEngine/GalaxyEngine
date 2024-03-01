#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
#include "Model.h"

namespace GALAXY 
{
	namespace Wrapper { class OBJLoader; class FBXLoader; }
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

			const char* GetResourceName() const override { return "Mesh"; }

			void Load() override;
			void Send() override;

			void Render(const Mat4& modelMatrix, const std::vector<Weak<class Material>>& materials, uint64_t id = -1) const;

			void DrawBoundingBox(const Component::Transform* transform) const;

			static inline ResourceType GetResourceType() { return ResourceType::Mesh; }

			static Path CreateMeshPath(const Path& modelPath, const Path& fileName);
			inline Resource::BoundingBox GetBoundingBox() const { return m_boundingBox; }

			Model* GetModel() const { return m_model; }

			bool HasModel() const { return !m_model; }

			Utils::Event<> OnLoad;
		private:
			void ComputeBoundingBox(const std::vector<Vec3f>& positionVertices);

		private:
			friend Wrapper::OBJLoader;
			friend Wrapper::FBXLoader;
			friend class Model;

			BoundingBox m_boundingBox;

			uint32_t m_vertexArrayIndex = -1;
			uint32_t m_vertexBufferIndex = -1;
			uint32_t m_indexBufferIndex = -1;

			Model* m_model = nullptr;

			std::vector<Vec3i> m_indices;
			std::vector<float> m_finalVertices;
			std::vector<SubMesh> m_subMeshes;
		};
	}
}
