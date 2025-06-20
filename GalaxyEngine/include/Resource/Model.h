#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
#include "Utils/Event.h"

namespace GALAXY {
	namespace Render { class Camera; }
	namespace Component { class Transform; }
	namespace Wrapper { class OBJLoader; class FBXLoader; }
	namespace Core { class GameObject; }
	namespace Physic {
		struct AABB;
		struct Plane; }
	namespace Resource
	{
		enum class ModelExtension
		{
			OBJ,
			FBX
		};

		struct BoundingBox
		{
			BoundingBox() : min(FLT_MAX), max(-FLT_MAX) {}
			BoundingBox(Vec3f min, Vec3f max) : min(min), max(max) {}
			BoundingBox(const Physic::AABB& aabb);

			Vec3f min;
			Vec3f max;

			Vec3f GetCenter() const;
			Vec3f GetExtents() const;

			bool IsOnFrustum(Render::Camera* camera, const Component::Transform* objectTransform) const;
			bool isOnOrForwardPlane(const Physic::Plane& plane) const;
		};
		class Material;
		class Mesh;
		class Model : public IResource
		{
		public:
			explicit Model(const Path& fullPath) : IResource(fullPath) {}
			Model& operator=(const Model& other) = default;
			Model(const Model&) = default;
			Model(Model&&) noexcept = default;
			~Model() override;

			bool Load() override;
			void Unload() override;

			void Send() override;

			const char* GetResourceName() const override { return "Default Resource"; }

			static inline ResourceType GetResourceType() { return ResourceType::Model; }
		
			inline BoundingBox GetBoundingBox() const { return m_boundingBox; }

			Shared<Core::GameObject> ToGameObject();

#ifdef WITH_EDITOR
			EDITOR_ONLY Path GetThumbnailPath() const override;
			
			EDITOR_ONLY void CreateThumbnail() override;
#endif
			
			void DrawBoundingBox(const Component::Transform* transform) const;

			List<Weak<Material>> GetMaterialsOfMesh(const class Mesh* mesh) const;
			
			const std::vector<Weak<Material>>& GetMaterials() const { return m_materials; }
			const std::vector<Weak<Mesh>>& GetMeshes() const { return m_meshes; }
			
		private:
			void ComputeBoundingBox(const std::vector<std::vector<Vec3f>>& positionVertices);

			void Serialize(CppSer::Serializer& serializer) const override;
			void Deserialize(CppSer::Parser& parser) override;

#ifdef WITH_EDITOR
			EDITOR_ONLY void ShowInInspector() override;
#endif
		private:
			// Note : the model is set to loaded only if all the mesh are loaded

			friend Wrapper::OBJLoader;
			friend Wrapper::FBXLoader;

			std::vector<Weak<class Mesh>> m_meshes;
			std::vector<Weak<class Material>> m_materials;

			BoundingBox m_boundingBox;

			ModelExtension m_modelType = ModelExtension::OBJ;

			std::atomic_bool m_meshesAdded = false;
		};
	}
}
