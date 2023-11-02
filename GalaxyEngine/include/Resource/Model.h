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

		class Model : public IResource
		{
		public:
			Model(const Path& fullPath) : IResource(fullPath) {}
			Model& operator=(const Model& other) = default;
			Model(const Model&) = default;
			Model(Model&&) noexcept = default;
			virtual ~Model();

			void Load() override;

			static inline ResourceType GetResourceType() { return ResourceType::Model; }
		private:
			void CreateDataFiles();

		private:

			friend Wrapper::OBJLoader;

			std::vector<Weak<class Mesh>> m_meshes;

			ModelExtension m_modelType = ModelExtension::OBJ;
		};
	}
}