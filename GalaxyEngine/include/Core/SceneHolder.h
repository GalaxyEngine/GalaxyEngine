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
		class SceneHolder
		{
		public:
			~SceneHolder();

			static SceneHolder* GetInstance();

			void Update();

			inline void SwitchScene(const Weak<Resource::Scene>& scene);

			static Resource::Scene* GetCurrentScene();

			void Release();
		private:
			void SwitchSceneUpdate();

			void SetCurrentScene(Shared<Resource::Scene> scene) { m_currentScene = scene; }
		private:
			friend Editor::ThumbnailCreator;

			static Unique<SceneHolder> m_instance;

			Shared<Resource::Scene> m_currentScene;

			Shared<Resource::Scene> m_nextScene;
		};
	}
}
#include "Core/SceneHolder.inl" 
