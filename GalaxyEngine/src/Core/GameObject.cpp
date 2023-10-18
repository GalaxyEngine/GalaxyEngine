#include "pch.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

#include "Utils/Parser.h"

#include "Component/ComponentHolder.h"

using namespace Core;
namespace GALAXY
{
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
		std::vector<std::weak_ptr<GameObject>> weakPtrVector;
		for (const auto& sharedPtr : m_childs) {
			weakPtrVector.push_back(sharedPtr);
		}
		return weakPtrVector;
	}

	std::weak_ptr<GameObject> GameObject::GetChild(uint32_t index)
	{
		if (index < m_childs.size())
			return m_childs.at(index);
		return std::weak_ptr<GameObject>();
	}

	void GameObject::AddChild(std::shared_ptr<GameObject> child, uint32_t index /*= -1*/)
	{
		// Check if child is not null 
		// and if the gameObject is not already the parent of the new child
		if (!child)
			return;
		// Check if the object is already on the list
		if (std::count_if(m_childs.begin(), m_childs.end(), [&](const std::shared_ptr<GameObject>& c)
			{ return c == child; }) == 0)
		{
			if (index == -1)
				m_childs.push_back(child);
			else
				m_childs.insert(m_childs.begin() + index, child);
		}
		// Check if the current object is already a parent of the child
		if (child->m_parent.lock().get() != this)
		{
			child->SetParent(weak_from_this());
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
		m_parent.lock()->AddChild(shared_from_this());
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
			m_childs[i]->UpdateSelfAndChild();
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
			m_childs[i]->DrawSelfAndChild();
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
			if (m_childs[i].get() == child)
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
		uint32_t index = component->GetIndex();
		m_components.erase(m_components.begin() + index);
		for (uint32_t i = index; i < m_components.size(); i++)
		{
			m_components[i]->p_id = i;
		}
	}

	void GameObject::ChangeComponentIndex(uint32_t prevIndex, uint32_t newIndex)
	{
		if (prevIndex < m_components.size() && newIndex < m_components.size())
		{
			auto elementToMove = std::move(m_components[prevIndex]);
			m_components.erase(m_components.begin() + prevIndex);
			m_components.insert(m_components.begin() + newIndex, std::move(elementToMove));
			m_components[newIndex]->p_id = newIndex;
			m_components[prevIndex]->p_id = prevIndex;
		}
	}

	Component::BaseComponent* GameObject::GetComponentWithName(const std::string& componentName)
	{
		for (auto component : m_components)
		{
			auto names = component->GetComponentNames();
			for (auto& curComponentName : names)
			{
				if (componentName == curComponentName) {
					return component.get();
				}
			}
		}
		return nullptr;
	}

	Weak<Component::BaseComponent> GameObject::GetComponentWithIndex(uint32_t index)
	{
		if (index < m_components.size())
		{
			return m_components[index];
		}
		return {};
	}

	void GameObject::RemoveFromParent()
	{
		if (!m_parent.lock())
			return;
		m_parent.lock()->RemoveChild(this);
	}

	void GameObject::AfterLoad()
	{
		for (auto& component : m_components)
		{
			component->AfterLoad();
			component->p_tempComponentIDs.clear();
			component->p_tempGameObjectIDs.clear();
		}
		for (auto& children : m_childs)
		{
			children->AfterLoad();
		}
	}

	std::vector<Weak<Core::GameObject>> Core::GameObject::GetAllChildren()
	{
		std::vector<Weak<Core::GameObject>> childs = GetChildren();
		for (auto& child : m_childs)
		{
			std::vector<Weak<Core::GameObject>> newChild = child->GetAllChildren();
			childs.insert(childs.end(), newChild.begin(), newChild.end());
		}
		return childs;
	}

	void GameObject::Serialize(Utils::Serializer& serializer)
	{
		serializer << PAIR::BEGIN_MAP << "BEGIN GAMEOBJECT";

		serializer << PAIR::KEY << "Name" << PAIR::VALUE << m_name;
		serializer << PAIR::KEY << "ID" << PAIR::VALUE << m_id;

		serializer << PAIR::KEY << "Component Number" << PAIR::VALUE << m_components.size();
		serializer << PAIR::KEY << "Child Number" << PAIR::VALUE << m_childs.size();

		serializer << PAIR::BEGIN_TAB;
		for (auto& components : m_components)
		{
			serializer << PAIR::BEGIN_MAP << "BEGIN COMPONENT";
			serializer << PAIR::KEY << "Name" << PAIR::VALUE << components->GetComponentName();
			components->Serialize(serializer);
			serializer << PAIR::END_MAP << "END COMPONENT";
		}
		serializer << PAIR::END_TAB;

		serializer << PAIR::BEGIN_TAB;
		for (auto& child : m_childs)
		{
			child->Serialize(serializer);
		}
		serializer << PAIR::END_TAB;

		serializer << PAIR::END_MAP << "END GAMEOBJECT";
	}

	void GameObject::Deserialize(Utils::Parser& parser)
	{
		m_name = parser["Name"];
		m_id = parser["ID"].As<uint64_t>();

		size_t componentNumber = parser["Component Number"].As<size_t>();
		m_childs.resize(parser["Child Number"].As<size_t>());

		for (size_t i = 0; i < componentNumber; i++)
		{
			parser.NewDepth();
			Shared<Component::BaseComponent> component;
			std::string componentNameString = parser["Name"];
			const char* componentName = componentNameString.c_str();
			for (auto& componentInstance : Component::ComponentHolder::GetList())
			{
				if (!strcmp(componentInstance->GetComponentName(), componentName))
				{
					component = componentInstance->Clone();
					break;
				}
			}
			if (component) {
				component->Deserialize(parser);
				AddComponent(component);
			}
		}

		for (auto& child : m_childs)
		{
			parser.NewDepth();

			child = std::make_shared<GameObject>();
			child->m_scene = m_scene;
			child->SetParent(weak_from_this());
			child->Deserialize(parser);
			m_scene->AddObject(child);
		}
	}
}