#pragma once
#include "GalaxyAPI.h"
#include <memory>
namespace GALAXY {
	namespace Editor
	{
		class ThumbnailCreator;
	}
	namespace Resource
	{
		class Scene;
	}
	namespace Core {
		class GALAXY_API SceneHolder
		{
		public:
			~SceneHolder();

			static SceneHolder* GetInstance();

			void Update();
			
			static void OpenScene(const std::filesystem::path& path);
			
			static void NewScene();

			inline void SwitchScene(const Weak<Resource::Scene>& scene);

#ifdef WITH_EDITOR
			inline void SwitchPlayModeScene();
#endif

			static Resource::Scene* GetCurrentScene();

			void Release();
		private:
			void SwitchSceneUpdate();

			void SetCurrentScene(const Shared<Resource::Scene>& scene) { m_currentScene = scene; }
		private:
			friend Editor::ThumbnailCreator;

			static Unique<SceneHolder> m_instance;

			Shared<Resource::Scene> m_currentScene;

			Shared<Resource::Scene> m_nextScene;

#ifdef WITH_EDITOR
			bool m_loadAfterEndPlay = false;
			
			bool first = true;
#endif
		};
	}
}
#include "Core/SceneHolder.inl" 
