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
			void Release();

			void AddToQueue(const Weak<Resource::IResource>& material);

			void CreateModelThumbnail(const Weak<Resource::Model>& model);

			auto CreateMaterialThumbnail(const Weak<Resource::Material>& material) -> void;

			void Update();

			static std::filesystem::path GetThumbnailPath(const Core::UUID& uuid);
			static bool IsThumbnailUpToDate(Resource::IResource* resource);
		private:
			void SaveThumbnail(const std::filesystem::path& thumbnailPath, const Vec2i& frameBufferSize);

		private:
			Vec2i m_thumbnailSize = Vec2i(512);
			bool m_initialized = false;

			std::deque<std::filesystem::path> m_thumbnailQueue;

			Shared<Core::GameObject> m_sphereMaterialObject = nullptr;
			Shared<Core::GameObject> m_cameraObject = nullptr;
			Shared<Component::CameraComponent> m_camera;
			Shared<Resource::Scene> m_scene;
		};
	}
}
