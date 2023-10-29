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
		class Camera;
		class EditorCamera;
		class Grid;
	}
	namespace Editor
	{
		class Gizmo;
		class ActionManager;
	}
	namespace Resource {
		class Scene : public IResource
		{
		public:
			Scene(const std::filesystem::path& path);
			Scene& operator=(const Scene& other) = default;
			Scene(const Scene&) = default;
			Scene(Scene&&) noexcept = default;
			virtual ~Scene();

#pragma region Resource Methods
			void Load() override;
			void Send() override;
			void Save(const std::filesystem::path& fullPath = "");

			static Weak<Scene> Create(const std::filesystem::path& path);

			static ResourceType GetResourceType() { return ResourceType::Scene; }
#pragma endregion

			void Initialize();

			std::weak_ptr<Core::GameObject> GetRootGameObject() const;

			// Every GameObject should be create via this function
			template<typename... Args> inline std::weak_ptr<Core::GameObject> CreateObject(Args&&... args);

			void AddObject(std::shared_ptr<Core::GameObject> gameObject);

			void RemoveObject(Core::GameObject* object);

			std::weak_ptr<Core::GameObject> GetWithIndex(uint64_t index);

			uint64_t GetFreeIndex();

			void Update();

			void SetCurrentCamera(std::weak_ptr<Render::Camera> camera);

			Mat4& GetVP() { return m_VP; }
			Shared<Render::EditorCamera> GetEditorCamera() const { return m_editorCamera; }
			std::weak_ptr<Render::Camera> GetCurrentCamera() const { return m_currentCamera; }

			void Release();

			inline Shared<Editor::Gizmo> GetGizmo() { return m_gizmo; }

			inline Shared<Editor::ActionManager> GetActionManager() { return m_actionManager; }

		private:
			Shared<Render::EditorCamera> m_editorCamera;
			std::weak_ptr<Render::Camera> m_currentCamera;
			Mat4 m_VP;

			Shared<Core::GameObject> m_root;
			std::unordered_map<uint64_t, Shared<Core::GameObject>> m_objectList;

			Shared<Editor::ActionManager> m_actionManager;

			Shared<Render::Grid> m_grid;
			Shared<Editor::Gizmo> m_gizmo;
		};
	}
}
#include "Resource/Scene.inl" 
