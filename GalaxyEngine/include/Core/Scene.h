#pragma once
#include "GalaxyAPI.h"
namespace GALAXY::Core {
	class Scene
	{
	public:
		Scene();
		Scene& operator=(const Scene& other) = default;
		Scene(const Scene&) = default;
		Scene(Scene&&) noexcept = default;
		virtual ~Scene();

		static Scene* GetInstance();

		void Update();
	private:

		static std::unique_ptr<Scene> m_instance;

		std::shared_ptr<class GameObject> m_root;
	};
}
