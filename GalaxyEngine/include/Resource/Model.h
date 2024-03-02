#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
#include "Utils/Event.h"

namespace GALAXY {
	namespace Wrapper { class OBJLoader; class FBXLoader; }
	namespace Core { class GameObject; }
	namespace Resource
	{
		enum class ModelExtension
		{
			OBJ,
			FBX
		};

		struct BoundingBox
		{
			Vec3f min;
			Vec3f max;
			Vec3f center;
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

			void CreateThumbnail();

			const char* GetResourceName() const override { return "Default Resource"; }

			static inline ResourceType GetResourceType() { return ResourceType::Model; }
		
			inline Resource::BoundingBox GetBoundingBox() const { return m_boundingBox; }

			Shared<Core::GameObject> ToGameObject();

			Utils::Event<> OnLoad;
		private:
			void ComputeBoundingBox(const std::vector<std::vector<Vec3f>>& positionVertices);

			void Serialize(CppSer::Serializer& serializer) const override;
			void Deserialize(CppSer::Parser& parser) override;

			void OnMeshLoaded();
		private:

			friend Wrapper::OBJLoader;
			friend Wrapper::FBXLoader;

			std::vector<Weak<class Mesh>> m_meshes;
			std::vector<Weak<class Material>> m_materials;

			BoundingBox m_boundingBox;

			ModelExtension m_modelType = ModelExtension::OBJ;
		};
	}
}
