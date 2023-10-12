#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"

namespace GALAXY
{
	namespace Core
	{
		class Scene;
		class GameObject;
	}
	namespace Resource
	{
		class SceneResource : public IResource
		{
		public:
			SceneResource(const std::filesystem::path& path) : IResource(path) {}
			SceneResource& operator=(const SceneResource& other) = default;
			SceneResource(const SceneResource&) = default;
			SceneResource(SceneResource&&) noexcept = default;
			virtual ~SceneResource() {}

			void Load() override;

			void Save();

			static Weak<SceneResource> Create(const std::filesystem::path& path);

			static ResourceType GetResourceType() { return ResourceType::Scene; }

			void Reload();

		private:
			friend Core::Scene;

			std::shared_ptr<Core::GameObject> m_root;
			std::vector<std::shared_ptr<Core::GameObject>> m_objects;
		};
	}
}
