#pragma once
#include "Component/ComponentHolder.h"

template<typename T>
inline void Component::ComponentHolder::RegisterComponent()
{
	m_componentList.push_back(std::make_shared<T>());
}

template<typename T>
inline void Component::ComponentHolder::RegisterComponent(T* component)
{
	m_componentList.push_back(Shared<T>(component));
}

template<typename T>
inline void Component::ComponentHolder::UnregisterComponent(T* component)
{
	for (size_t i = 0; i < m_componentList.size(); i++)
	{
		if (m_componentList[i].get() == component)
		{
			m_componentList.erase(m_componentList.begin() + i);
			return;
		}
	}
}