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

		Scene* GetCurrentScene();

	private:
		static std::unique_ptr<SceneHolder> m_instance;

		std::shared_ptr<Scene> m_currentScene;
	};
}
