#pragma once
#include "GalaxyAPI.h"
#include <memory>
namespace GALAXY::Core {
	class Scene;
	class SceneHolder
	{
	public:
		~SceneHolder();

		static SceneHolder* GetInstance();

		void Update();

		static Scene* GetCurrentScene();

		void Release();

	private:
		static std::unique_ptr<SceneHolder> m_instance;

		std::shared_ptr<Scene> m_currentScene;
	};
}
