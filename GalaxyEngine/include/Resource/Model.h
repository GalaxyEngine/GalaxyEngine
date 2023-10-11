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
			Model(const std::filesystem::path& fullPath) : IResource(fullPath) {}
			Model& operator=(const Model& other) = default;
			Model(const Model&) = default;
			Model(Model&&) noexcept = default;
			virtual ~Model();

			void Load() override;

			static ResourceType GetResourceType() { return ResourceType::Model; }
		private:
			void CreateFiles();

		private:

			friend Wrapper::OBJLoader;

			std::vector<std::shared_ptr<class Mesh>> m_meshes;

			ModelExtension m_modelType = ModelExtension::OBJ;
		};
	}
}