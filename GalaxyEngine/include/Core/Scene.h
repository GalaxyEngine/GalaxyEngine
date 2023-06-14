#pragma once
#include "GalaxyAPI.h"
#include "GameObject.h"
#include <unordered_map>

namespace GALAXY::Core {
	class Scene
	{
	public:
		Scene();
		Scene& operator=(const Scene& other) = default;
		Scene(const Scene&) = default;
		Scene(Scene&&) noexcept = default;
		virtual ~Scene();

		std::weak_ptr<GameObject> GetRootGameObject() const;

		// Every GameObject should be create via this function
		template<typename... Args> std::weak_ptr<GameObject> CreateObject(Args&&... args)
		{
			std::shared_ptr<GameObject> shared = std::make_shared<GameObject>(std::forward<Args>(args)...);
			shared->Initialize();
			uint64_t objectIndex = GetFreeIndex();
			shared->m_id = objectIndex;
			m_objectList[objectIndex] = shared;
			return shared;
		}

		void RemoveObject(GameObject* object);

		std::weak_ptr<GameObject> GetWithIndex(uint64_t index);

		uint64_t GetFreeIndex();

		void Update();
	private:

		std::shared_ptr<GameObject> m_root;
		std::unordered_map<uint64_t, std::shared_ptr<GameObject>> m_objectList;
	};
}
