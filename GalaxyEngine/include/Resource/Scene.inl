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

		m_lastAdded.push_back(shared);

		return shared;
	}

	inline void Resource::Scene::AddObject(std::shared_ptr<Core::GameObject> gameObject)
	{
		if (!m_objectList.count(gameObject->m_id))
		{
			m_objectList[gameObject->m_id] = gameObject;
		}
		else
		{
			gameObject->m_id = GetFreeIndex();
			m_root->AddChild(gameObject);
		}
	}

	inline void Resource::Scene::RemoveObject(Core::GameObject* object)
	{
		auto shared = std::find_if(m_objectList.begin(), m_objectList.end(), [&](const std::pair<uint64_t, std::shared_ptr<Core::GameObject>>& element) {
			return element.second.get() == object; });
		if (shared != m_objectList.end()) {
			shared->second->RemoveFromParent();
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
			last.lock()->RemoveFromParent();
			m_lastAdded.pop_back();
		}
	}

	inline uint64_t Resource::Scene::GetFreeIndex()
	{
		uint64_t index = 0;
		while (m_objectList.find(index) != m_objectList.end()) {
			index++;
		}
		return index;
	}

	inline Weak<Core::GameObject> Resource::Scene::GetWithIndex(uint64_t index)
	{
		if (m_objectList.count(index))
		{
			return m_objectList.at(index);
		}
		return Weak<Core::GameObject>();
	}
}
