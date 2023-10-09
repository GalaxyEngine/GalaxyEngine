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
			Mesh(const std::filesystem::path& fullPath);
			Mesh& operator=(const Mesh& other) = default;
			Mesh(const Mesh&) = default;
			Mesh(Mesh&&) noexcept = default;
			virtual ~Mesh() {}

			void Load() override;
			void Send() override;

			void Render(const Mat4& modelMatrix, const std::vector<std::weak_ptr<class Material>>& materials, uint64_t id = -1);

			static ResourceType GetResourceType() { return ResourceType::Mesh; }

			static std::filesystem::path CreateMeshPath(const std::filesystem::path& modelPath, const std::filesystem::path& fileName);

		private:
			friend Wrapper::OBJLoader;
			friend class Model;

			std::filesystem::path m_modelPath;

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
