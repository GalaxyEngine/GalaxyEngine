#include "pch.h"

#include "Utils/Define.h"

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

	GameObject::GameObject(String name) : GameObject()
	{
		SetName(name);
	}

	Core::GameObject::~GameObject()
	{
		PrintError("GameObject \"%s\" deleted !", m_name.c_str());
	}

	void GameObject::Destroy()
	{
		for (size_t i = 0; i < m_components.size(); i++)
		{
			m_components[i]->RemoveFromGameObject();
		}
		for (size_t i = 0; i < m_childs.size(); i++)
		{
			m_childs[i]->Destroy();
		}
		RemoveFromParent();
		GetScene()->RemoveObject(this);
	}

	void GameObject::AddChild(Shared<GameObject> child, uint32_t index /*= -1*/)
	{
		// Check if child is not null 
		// and if the gameObject is not already the parent of the new child
		if (!child)
			return;
		// Check if the object is already on the list
		if (std::count_if(m_childs.begin(), m_childs.end(), [&](const Shared<GameObject>& c)
			{ return c == child; }) == 0)
		{
			if (index == INDEX_NONE)
				m_childs.push_back(child);
			else
				m_childs.insert(m_childs.begin() + index, child);
		}
		// Check if the current object is already a parent of the child
		if (child->m_parent.lock().get() != this)
		{
			child->SetParent(Weak<GameObject>(weak_from_this()));
		}

	}

	void GameObject::SetParent(Weak<GameObject> parent)
	{
		if (!parent.lock())
			return;
		if (Shared<GameObject> prevParent = m_parent.lock())
		{
			prevParent->RemoveChild(this);
		}
		m_parent = parent;
		m_parent.lock()->AddChild(shared_from_this());
	}

	void GameObject::RemoveChild(GameObject* child)
	{
		m_childs.erase(std::remove_if(m_childs.begin(), m_childs.end(), [&](const Weak<GameObject>& c) {
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
		m_transform->SetGameObject(shared_from_this());
	}

	void GameObject::UpdateSelfAndChild()
	{
		m_transform->OnUpdate();
		for (uint32_t i = 0; i < m_components.size(); i++)
		{
			if (m_components[i]->IsEnable()) {
				m_components[i]->OnUpdate();
				m_components[i]->OnEditorUpdate();
			}
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
			if (m_components[i]->IsEnable()) {
				m_components[i]->OnDraw();
				m_components[i]->OnEditorDraw();
			}
		}

		for (uint32_t i = 0; i < m_childs.size(); i++)
		{
			m_childs[i]->DrawSelfAndChild();
		}
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

	bool GameObject::IsSibling(const List<Weak<GameObject>>& siblings)
	{
		for (size_t i = 0; i < siblings.size(); i++)
		{
			if (siblings[i].lock()->GetParent() != this->GetParent())
				return false;
		}
		return true;
	}

	void GameObject::RemoveComponent(Component::BaseComponent* component)
	{
		component->OnDestroy();
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

	Component::BaseComponent* GameObject::GetComponentWithName(const String& componentName)
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

	List<Weak<Core::GameObject>> Core::GameObject::GetAllChildren()
	{
		List<Weak<Core::GameObject>> childs = GetChildren();
		for (auto& child : m_childs)
		{
			List<Weak<Core::GameObject>> newChild = child->GetAllChildren();
			childs.insert(childs.end(), newChild.begin(), newChild.end());
		}
		return childs;
	}

	void SerializeTransform(Utils::Serializer& serializer, Component::Transform* transform)
	{
		serializer << Pair::BEGIN_MAP << "BEGIN TRANSFORM";
		transform->Serialize(serializer);
		serializer << Pair::END_MAP << "END TRANSFORM";
	}

	void SerializeComponent(Utils::Serializer& serializer, Shared<Component::BaseComponent> components)
	{
		serializer << Pair::BEGIN_MAP << "BEGIN COMPONENT";
		serializer << Pair::KEY << "Name" << Pair::VALUE << components->GetComponentName();
		serializer << Pair::KEY << "Enable" << Pair::VALUE << components->IsEnable();
		components->Serialize(serializer);
		serializer << Pair::END_MAP << "END COMPONENT";
	}

	void GameObject::Serialize(Utils::Serializer& serializer)
	{
		serializer << Pair::BEGIN_MAP << "BEGIN GAMEOBJECT";

		serializer << Pair::KEY << "Name"	<< Pair::VALUE << m_name;
		serializer << Pair::KEY << "Active" << Pair::VALUE << m_active;
		serializer << Pair::KEY << "UUID"	<< Pair::VALUE << m_UUID;

		serializer << Pair::KEY << "Component Number" << Pair::VALUE << m_components.size();
		serializer << Pair::KEY << "Child Number"	  << Pair::VALUE << m_childs.size();

		SerializeTransform(serializer, m_transform.get());

		serializer << Pair::BEGIN_TAB;
		for (Shared<Component::BaseComponent>& component : m_components)
		{
			SerializeComponent(serializer, component);
		}
		serializer << Pair::END_TAB;

		serializer << Pair::BEGIN_TAB;
		for (auto& child : m_childs)
		{
			child->Serialize(serializer);
		}
		serializer << Pair::END_TAB;

		serializer << Pair::END_MAP << "END GAMEOBJECT";
	}

	void GameObject::Deserialize(Utils::Parser& parser, bool parseUUID /*= true*/)
	{
		m_name = parser["Name"];
		m_active = parser["Active"].As<bool>();
		if (parseUUID)
			m_UUID = parser["UUID"].As<uint64_t>();

		size_t componentNumber = parser["Component Number"].As<size_t>();
		m_childs.resize(parser["Child Number"].As<size_t>());

		parser.NewDepth();
		m_transform->Deserialize(parser);

		for (size_t i = 0; i < componentNumber; i++)
		{
			parser.NewDepth();
			Shared<Component::BaseComponent> component;
			String componentNameString = parser["Name"];
			bool enable = parser["Enable"].As<bool>();
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
				component->SetSelfEnable(enable);
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
			child->Deserialize(parser, parseUUID);
			m_scene->AddObject(child);
		}
	}
}