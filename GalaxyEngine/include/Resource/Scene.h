#pragma once
#include "GalaxyAPI.h"

#include "Resource/IResource.h"

#include "Core/GameObject.h"

#include <unordered_map>

namespace GALAXY {
	namespace Resource
	{
		class SceneResource;
	}
	namespace Render {
		class LightManager;
		class Camera;
		class EditorCamera;
		class Grid;
	}
	namespace Editor
	{
		class Gizmo;
		class ActionManager;
	}
	namespace Component
	{
		class CameraComponent;
	}

	enum class DrawMode
	{
		None,
		Editor,
		Game,
	};

	namespace Resource {
		class Scene : public IResource
		{
		public:
			explicit Scene(const Path& path);
			Scene& operator=(const Scene& other) = default;
			Scene(const Scene&) = default;
			Scene(Scene&&) noexcept = default;
			~Scene() override;

#pragma region Resource Methods
			void Load() override;
			void Unload() override;
			void Send() override;
			void Save(const Path& fullPath = "") const;

			const char* GetResourceName() const override { return "Scene"; }

			static Weak<Scene> Create(const Path& path);

			static inline ResourceType GetResourceType() { return ResourceType::Scene; }
#pragma endregion

			void Initialize();

			void Update();

			// Every GameObject should be create via this function
			template<typename... Args>
			inline Weak<Core::GameObject> CreateObject(Args&&... args);

			inline void AddObject(const std::shared_ptr<Core::GameObject>& gameObject);

			inline void RemoveObject(Core::GameObject* object);

			void SetCurrentCamera(const Weak<Render::Camera>& camera);

			inline void RevertObject(size_t number = 1);

			// Call when the window should close to prevent unsaved scene
			bool WasModified() const;

			void AddCamera(const Weak<Component::CameraComponent>& camera);
			void SetMainCamera(const Weak<Component::CameraComponent>& camera);

			// Return the GameObject with the index given in scene Graph
			inline Weak<Core::GameObject> GetWithSceneGraphID(uint64_t index);
			// Return the GameObject with the uuid given
			inline Weak<Core::GameObject> GetWithUUID(const Core::UUID& uuid);
			inline Weak<Core::GameObject> GetRootGameObject() const;
			inline const Mat4& GetVP() const;
			inline const Vec3f& GetCameraUp() const;
			inline const Vec3f& GetCameraRight() const;
			inline Shared<Render::EditorCamera> GetEditorCamera() const;
			inline Shared<Render::Camera> GetCurrentCamera() const;
			inline Shared<Component::CameraComponent> GetMainCamera() const;
			inline Shared<Editor::Gizmo> GetGizmo() const;
			inline Shared<Editor::ActionManager> GetActionManager() const;

			inline const UMap<Core::UUID, Shared<Core::GameObject>>& GetObjectList() const;

			Shared<Render::LightManager> GetLightManager() const { return m_lightManager; }
		private:
			List<Weak<Component::CameraComponent>> m_cameras;
			Shared<Render::EditorCamera> m_editorCamera;
			Weak<Render::Camera> m_currentCamera;
			Weak<Component::CameraComponent> m_mainCamera;
			Shared<Render::LightManager> m_lightManager = nullptr;

			Mat4 m_VP;
			Vec3f m_cameraUp;
			Vec3f m_cameraRight;

			Shared<Core::GameObject> m_root;

			//Core::ECSystem* m_ecSystem;

			UMap<Core::UUID, Shared<Core::GameObject>> m_objectList;

			Shared<Editor::ActionManager> m_actionManager;

			Shared<Render::Grid> m_grid;
			Shared<Editor::Gizmo> m_gizmo;

			List<Weak<Core::GameObject>> m_lastAdded;

		};
	}
}
#include "Resource/Scene.inl" 
