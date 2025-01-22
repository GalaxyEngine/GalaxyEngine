#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
#include "Utils/Event.h"

namespace GALAXY {
	namespace Render { class Camera; }
	namespace Component { class Transform; }
	namespace Wrapper { class OBJLoader; class FBXLoader; }
	namespace Core { class GameObject; }
	namespace Physic { struct Plane; }
	namespace Resource
	{
		enum class ModelExtension
		{
			OBJ,
			FBX
		};

		struct BoundingBox
		{
			BoundingBox() : min(FLT_MAX), max(FLT_MIN) {}
			BoundingBox(Vec3f min, Vec3f max) : min(min), max(max) {}

			Vec3f min = Vec3f(FLT_MAX);
			Vec3f max = Vec3f(FLT_MIN);

			Vec3f GetCenter() const;
			Vec3f GetExtents() const;

			bool IsOnFrustum(Render::Camera* camera, Component::Transform* objectTransform) const;
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

			void Load() override;
			void Unload() override;

			const char* GetResourceName() const override { return "Default Resource"; }
#ifdef WITH_EDITOR
			EDITOR_ONLY Path GetThumbnailPath() const override;
#endif

			static inline ResourceType GetResourceType() { return ResourceType::Model; }
		
			inline BoundingBox GetBoundingBox() const { return m_boundingBox; }

			Shared<Core::GameObject> ToGameObject();

#ifdef WITH_EDITOR
			EDITOR_ONLY void CreateThumbnail();
#endif
			
			void DrawBoundingBox(const Component::Transform* transform) const;

			List<Weak<Material>> GetMaterialsOfMesh(const class Mesh* mesh) const;
			
			const std::vector<Weak<Material>>& GetMaterials() const { return m_materials; }
			const std::vector<Weak<Mesh>>& GetMeshes() const { return m_meshes; }
		public:
			Utils::Event<> EOnLoad;
		private:
			void ComputeBoundingBox(const std::vector<std::vector<Vec3f>>& positionVertices);

			void Serialize(CppSer::Serializer& serializer) const override;
			void Deserialize(CppSer::Parser& parser) override;

			EDITOR_ONLY void ShowInInspector() override;

			void OnMeshLoaded();
		private:
			// Note : the model is set to loaded only if all the mesh are loaded

			friend Wrapper::OBJLoader;
			friend Wrapper::FBXLoader;

			std::vector<Weak<class Mesh>> m_meshes;
			std::vector<Weak<class Material>> m_materials;

			BoundingBox m_boundingBox;

			ModelExtension m_modelType = ModelExtension::OBJ;
		};
	}
}
