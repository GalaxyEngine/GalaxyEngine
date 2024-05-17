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
			Path GetThumbnailPath() const override;

			static inline ResourceType GetResourceType() { return ResourceType::Model; }
		
			inline Resource::BoundingBox GetBoundingBox() const { return m_boundingBox; }

			Shared<Core::GameObject> ToGameObject();

			Utils::Event<> OnLoad;

#ifdef WITH_EDITOR
			void CreateThumbnail();
#endif
			const std::vector<Weak<class Material>>& GetMaterials() const { return m_materials; }
			const std::vector<Weak<class Mesh>>& GetMeshes() const { return m_meshes; }
		private:
			void ComputeBoundingBox(const std::vector<std::vector<Vec3f>>& positionVertices);

			void Serialize(CppSer::Serializer& serializer) const override;
			void Deserialize(CppSer::Parser& parser) override;

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
