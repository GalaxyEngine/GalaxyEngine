#pragma once
#include "Resource/Scene.h"
namespace GALAXY 
{
	template<typename... Args> inline std::weak_ptr<Core::GameObject> Resource::Scene::CreateObject(Args&&... args)
	{
		std::shared_ptr<Core::GameObject> shared = std::make_shared<Core::GameObject>(std::forward<Args>(args)...);
		shared->Initialize();
		uint64_t objectIndex = GetFreeIndex();
		shared->m_scene = this;
		shared->m_id = objectIndex;
		m_objectList[objectIndex] = shared;
		return shared;
	}
}
