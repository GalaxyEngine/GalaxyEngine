#pragma once
#include "GalaxyAPI.h"
#include <memory>
namespace GALAXY {
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

			void SwitchScene(Weak<Resource::Scene> scene);

			static Resource::Scene* GetCurrentScene();

			void Release();
		private:
			void SwitchSceneUpdate();
		private:
			static std::unique_ptr<SceneHolder> m_instance;

			Shared<Resource::Scene> m_currentScene;

			Shared<Resource::Scene> m_nextScene;
		};
	}
}