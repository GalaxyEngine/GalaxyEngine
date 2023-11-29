#pragma once
#include "Resource/Scene.h"
namespace GALAXY 
{
	template<typename... Args> inline Weak<Core::GameObject> Resource::Scene::CreateObject(Args&&... args)
	{
		std::shared_ptr<Core::GameObject> shared = std::make_shared<Core::GameObject>(std::forward<Args>(args)...);
		shared->Initialize();
		shared->m_scene = this;

		AddObject(shared);

		m_lastAdded.push_back(shared);

		return shared;
	}

	inline void Resource::Scene::AddObject(std::shared_ptr<Core::GameObject> gameObject)
	{
		if (!m_objectList.count(gameObject->m_UUID))
		{
			m_objectList[gameObject->m_UUID] = gameObject;
		}
		else
		{		
			ASSERT(false);
		}
	}

	inline void Resource::Scene::RemoveObject(Core::GameObject* object)
	{
		auto shared = std::find_if(m_objectList.begin(), m_objectList.end(), [&](const std::pair<uint64_t, std::shared_ptr<Core::GameObject>>& element) {
			return element.second.get() == object; });
		if (shared != m_objectList.end()) {
			if (shared != m_objectList.end())
			{
				m_objectList.erase(shared);
			}
		}
	}

	inline void Resource::Scene::RevertObject(size_t number)
	{
		for (size_t i = 0; i < number; ++i) 
		{
			auto last = m_lastAdded.back();
			last.lock()->Destroy();
			m_lastAdded.pop_back();
		}
	}

	inline Weak<GALAXY::Core::GameObject> Resource::Scene::GetWithSceneGraphID(uint64_t index)
	{
		for (auto& [id, object]:m_objectList)
		{
			if (object->GetSceneGraphID() == index)
			{
				return object;
			}
		}
		return {};
	}

	inline Weak<GALAXY::Core::GameObject> Resource::Scene::GetWithUUID(Core::UUID uuid)
	{
		if (m_objectList.count(uuid))
		{
			return m_objectList.at(uuid);
		}
		return {};
	}


}
