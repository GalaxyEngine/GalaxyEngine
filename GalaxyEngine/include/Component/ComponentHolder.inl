#pragma once
#include "Component/ComponentHolder.h"

template<typename T>
void Component::ComponentHolder::RegisterComponent()
{
	m_componentList.push_back(std::make_shared<T>());
}

template<typename T>
void Component::ComponentHolder::RegisterComponent(Shared<T> component)
{
	m_componentList.push_back(component);
}