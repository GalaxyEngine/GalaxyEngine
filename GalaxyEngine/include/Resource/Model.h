#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"
namespace GALAXY {
	namespace Wrapper { class OBJLoader; }
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

			const char* GetResourceName() const override { return "Default Resource"; }

			static inline ResourceType GetResourceType() { return ResourceType::Model; }
		
			inline Resource::BoundingBox GetBoundingBox() const { return m_boundingBox; }
		private:
			void ComputeBoundingBox();

			void Serialize(CppSer::Serializer& serializer) const override;
			void Deserialize(CppSer::Parser& parser) override;

		private:

			friend Wrapper::OBJLoader;

			std::vector<Weak<class Mesh>> m_meshes;

			BoundingBox m_boundingBox;

			ModelExtension m_modelType = ModelExtension::OBJ;
		};
	}
}