#pragma once
#include "GalaxyAPI.h"
#include <filesystem>
#include <queue>

namespace GALAXY
{
	namespace Component
	{
		class CameraComponent;
	}

	namespace Core
	{
		class UUID;
		class GameObject;
	}
	namespace Resource
	{
		class Scene;
		class IResource;
		class Material;
		class Model;
	}
	namespace Editor
	{
		class ThumbnailCreator
		{
		public:
			void Initialize();
			void AddToQueue(const Weak<Resource::Material>& material);

			void CreateThumbnail(Resource::Model* model);

			auto CreateMaterialThumbnail(const Weak<Resource::Material>& material) -> void;

			void Update();

			static std::filesystem::path GetThumbnailPath(const Core::UUID& uuid);
			static bool IsThumbnailUpToDate(Resource::IResource* resource);
		private:
			bool m_initialized = false;

			std::deque<std::filesystem::path> m_thumbnailQueue;

			Shared<Core::GameObject> m_sphereMaterialObject = nullptr;
			Shared<Core::GameObject> m_cameraObject = nullptr;
			Shared<Component::CameraComponent> m_camera;
			Shared<Resource::Scene> m_scene;
		};
	}
}
