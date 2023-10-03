#include "pch.h"
#include "Core/GameObject.h"

using namespace Core;

GameObject::GameObject()
{
	m_transform = std::make_unique<Component::Transform>();
}

GameObject::GameObject(const std::string& name) : GameObject()
{
	m_name = name;
}

std::vector<std::weak_ptr<GameObject>> GameObject::GetChildren()
{
	return m_childs;
}

std::weak_ptr<GameObject> GameObject::GetChild(uint32_t index)
{
	if (index < m_childs.size())
		return m_childs.at(index);
	return std::weak_ptr<GameObject>();
}

void GameObject::AddChild(std::weak_ptr<GameObject> child, uint32_t index /*= -1*/)
{
	// Check if child is not null 
	// and if the gameobject is not already the parent of the new child
	if (auto lockedChild = child.lock(); lockedChild) {
		// Check if the object is already on the list
		if (std::count_if(m_childs.begin(), m_childs.end(), [&](const std::weak_ptr<GameObject>& c)
			{ return c.lock() == lockedChild; }) == 0)
		{
			if (index == -1)
				m_childs.push_back(child);
			else
				m_childs.insert(m_childs.begin() + index, child);
		}
		// Check if the current object is already a parent of the child
		if (lockedChild->m_parent.lock().get() != this)
		{
			lockedChild->SetParent(weak_from_this());
		}
	}
}

std::weak_ptr<Core::GameObject> GameObject::GetParent()
{
	return m_parent;
}

void GameObject::SetParent(std::weak_ptr<GameObject> parent)
{
	if (!parent.lock())
		return;
	if (std::shared_ptr<GameObject> prevParent = m_parent.lock())
	{
		prevParent->RemoveChild(this);
	}
	m_parent = parent;
	m_parent.lock()->AddChild(weak_from_this());
}

void GameObject::RemoveChild(GameObject* child)
{
	m_childs.erase(std::remove_if(m_childs.begin(), m_childs.end(), [&](const std::weak_ptr<GameObject>& c) {
		return c.lock().get() == child;
		}), m_childs.end());
}

void GameObject::RemoveChild(uint32_t index)
{
	if (index < m_childs.size())
		m_childs.erase(m_childs.begin() + index);
}

void GameObject::Initialize()
{
	m_transform->gameObject = shared_from_this();
}

void GameObject::UpdateSelfAndChild()
{
	m_transform->OnUpdate();
	for (uint32_t i = 0; i < m_components.size(); i++)
	{
		if (m_components[i]->IsEnable())
			m_components[i]->OnUpdate();
	}

	for (uint32_t i = 0; i < m_childs.size(); i++)
	{
		if (m_childs[i].expired())
		{
			m_childs.erase(m_childs.begin() + i);
			i--;
		}
		else
		{
			m_childs[i].lock()->UpdateSelfAndChild();
		}
	}
}

void GameObject::DrawSelfAndChild()
{
	for (uint32_t i = 0; i < m_components.size(); i++)
	{
		if (m_components[i]->IsEnable())
			m_components[i]->OnDraw();
	}

	for (uint32_t i = 0; i < m_childs.size(); i++)
	{
		if (m_childs[i].expired())
		{
			m_childs.erase(m_childs.begin() + i);
			i--;
		}
		else
		{
			m_childs[i].lock()->DrawSelfAndChild();
		}
	}
}



bool GameObject::IsAParent(GameObject* object)
{
	if (object == this)
		return true;
	else if (m_parent.lock())
		return m_parent.lock()->IsAParent(object);
	return false;
}

uint32_t GameObject::GetChildIndex(GameObject* child)
{
	for (uint32_t i = 0; i < m_childs.size(); i++)
	{
		if (m_childs[i].lock().get() == child)
			return i;
	}
	// Not found
	return -1;
}

bool GameObject::IsSibling(const std::vector<std::weak_ptr<GameObject>>& siblings)
{
	for (size_t i = 0; i < siblings.size(); i++)
	{
		if (siblings[i].lock()->GetParent().lock() != this->GetParent().lock())
			return false;
	}
	return true;
}

void GameObject::RemoveComponent(Component::BaseComponent* component)
{
	for (size_t i = 0; i < m_components.size(); i++)
	{
		if (component == m_components[i].get())
		{
			m_components.erase(m_components.begin() + i);
			return;
		}
	}
}

uint32_t GameObject::GetComponentIndex(Component::BaseComponent* component)
{
	for (uint32_t i = 0; i < m_components.size(); i++)
	{
		if (m_components[i].get() == component)
			return i;
	}
	// Not found
	return -1;
}

void GameObject::ChangeComponentIndex(uint32_t prevIndex, uint32_t newIndex)
{
	if (prevIndex < m_components.size() && newIndex < m_components.size())
	{
		auto elementToMove = std::move(m_components[prevIndex]);
		m_components.erase(m_components.begin() + prevIndex);
		m_components.insert(m_components.begin() + newIndex, std::move(elementToMove));
	}
}

std::string GameObject::GetName() const
{
	return m_name;
}
