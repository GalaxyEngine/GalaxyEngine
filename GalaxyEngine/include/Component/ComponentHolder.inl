#pragma once
#include "Component/ComponentHolder.h"

template<typename T>
void Component::ComponentHolder::RegisterComponent()
{
	m_componentList.push_back(std::make_shared<T>());
}

template<typename T>
void Component::ComponentHolder::RegisterComponent(T* component)
{
	m_componentList.push_back(std::shared_ptr<T>(component));
}


template<typename T>
void Component::ComponentHolder::UnregisterComponent(T* component)
{
	for (int i = 0; i < m_componentList.size(); i++)
	{
		if (m_componentList[i].get() == component)
		{
			m_componentList.erase(m_componentList.begin() + i);
			return;
		}
	}
}