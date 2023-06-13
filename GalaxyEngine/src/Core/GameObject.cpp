#include "pch.h"
#include "Core/GameObject.h"

using namespace Core;
std::vector<std::weak_ptr<GameObject>> GameObject::GetChildren()
{
	std::vector<std::weak_ptr<GameObject>> weakPtrs;
	weakPtrs.reserve(m_childs.size());

	for (const auto& sharedPtr : m_childs) {
		weakPtrs.push_back(sharedPtr);
	}

	return weakPtrs;
}

std::weak_ptr<GameObject> GameObject::GetChild(uint32_t index)
{
	if (index < m_childs.size())
		return m_childs.at(index);
	return std::weak_ptr<GameObject>();
}

void GameObject::AddChild(GameObject* child)
{
	AddChild(std::shared_ptr<GameObject>(child));
}

void GameObject::AddChild(std::shared_ptr<GameObject> child)
{
	if (!std::count(m_childs.begin(), m_childs.end(), child)) {
		m_childs.push_back(child);
		child->SetParent(shared_from_this());
	}
}

std::weak_ptr<Core::GameObject> GameObject::GetParent()
{
	return m_parent;
}

void GameObject::SetParent(std::shared_ptr<GameObject> parent)
{
	if (std::shared_ptr<GameObject> prevParent = m_parent.lock())
	{
		prevParent->RemoveChild(shared_from_this());
	}
	m_parent = parent;
}

void GameObject::SetParent(GameObject* parent)
{
	SetParent(std::shared_ptr<GameObject>(parent));
}

void GameObject::RemoveChild(const std::shared_ptr<GameObject>& child)
{
	m_childs.erase(std::remove(m_childs.begin(), m_childs.end(), child), m_childs.end());
}

void GameObject::RemoveChild(uint32_t index)
{
	if (index < m_childs.size())
		m_childs.erase(m_childs.begin() + index);
}

