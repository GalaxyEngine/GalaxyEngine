#include "pch.h"

#include "Utils/Define.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

#include "Utils/Parser.h"

#include "Component/ComponentHolder.h"

using namespace Core;
using namespace Component;
namespace GALAXY
{
	GameObject::GameObject()
	{
		m_transform = std::make_unique<Component::Transform>();
	}

	GameObject::GameObject(const String& name) : GameObject()
	{
		SetName(name);
	}

	GameObject::~GameObject()
	{
		//PrintError("GameObject \"%s\" deleted !", m_name.c_str());
	}

	void GameObject::Destroy()
	{
		for (size_t i = 0; m_components.size(); i++)
		{
			m_components[i]->RemoveFromGameObject();
			i--;
		}
		for (size_t i = 0; i < m_children.size(); i++)
		{
			m_children[i]->Destroy();
			i--;
		}

		RemoveFromParent();
		GetScene()->RemoveObject(this);
	}

	void GameObject::AddChild(const Shared<GameObject>& child, const uint32_t index /*= -1*/)
	{
		// Check if child is not null 
		// and if the gameObject is not already the parent of the new child
		if (!child)
			return;
		// Check if the object is already on the list
		if (std::ranges::count_if(m_children, [&](const Shared<GameObject>& c)
			{ return c == child; }) == 0)
		{
			// if the index is not set, add the child at the end of the list
			if (index != INDEX_NONE)
				m_children.insert(m_children.begin() + index, child);
			else
				m_children.push_back(child);
		}
		// Check if the current object is already a parent of the child
		if (child->m_parent.lock().get() != this)
		{
			child->SetParent(weak_from_this());
		}

	}

	void GameObject::SetParent(const Weak<GameObject>& parent)
	{
		if (!parent.lock())
			return;
		if (const Shared<GameObject> prevParent = m_parent.lock())
		{
			prevParent->RemoveChild(this);
		}
		m_parent = parent;
		m_parent.lock()->AddChild(shared_from_this());
	}

	void GameObject::RemoveChild(const GameObject* child)
	{
		std::erase_if(m_children, [&](const Weak<GameObject>& c) {
			return c.lock().get() == child;
			});
	}

	void GameObject::RemoveChild(const uint32_t index)
	{
		if (index < m_children.size())
			m_children.erase(m_children.begin() + index);
	}

	void GameObject::Initialize()
	{
		m_transform->SetGameObject(shared_from_this());
	}

	void GameObject::UpdateSelfAndChild() const
	{
		m_transform->OnUpdate();
		for (const auto& m_component : m_components)
		{
			if (m_component->IsEnable()) {
				m_component->OnUpdate();
				m_component->OnEditorUpdate();
			}
		}

		for (const Shared<GameObject>& m_child : m_children)
		{
			m_child->UpdateSelfAndChild();
		}
	}

	void GameObject::DrawSelfAndChild() const
	{
		for (const auto& m_component : m_components)
		{
			if (m_component->IsEnable()) {
				m_component->OnDraw();
				m_component->OnEditorDraw();
			}
		}

		for (const auto& m_child : m_children)
		{
			m_child->DrawSelfAndChild();
		}
	}

	uint32_t GameObject::GetChildIndex(const GameObject* child) const
	{
		for (uint32_t i = 0; i < m_children.size(); i++)
		{
			if (m_children[i].get() == child)
				return i;
		}
		// Not found
		return -1;
	}

	bool GameObject::IsSibling(const List<Weak<GameObject>>& siblings) const
	{
		for (const Weak<GameObject>& sibling : siblings)
		{
			if (sibling.lock()->GetParent() != this->GetParent())
				return false;
		}
		return true;
	}

	void GameObject::RemoveComponent(Component::BaseComponent* component)
	{
		component->OnDestroy();
		const uint32_t index = component->GetIndex();
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

	Component::BaseComponent* GameObject::GetComponentWithName(const String& componentName) const
	{
		for (const Shared<Component::BaseComponent>& component : m_components)
		{
			List<const char*> names = component->GetComponentNames();
			for (const auto& curComponentName : names)
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

	void GameObject::RemoveFromParent() const
	{
		if (!m_parent.lock())
			return;
		m_parent.lock()->RemoveChild(this);
	}

	void GameObject::AfterLoad() const
	{
		for (const Shared<Component::BaseComponent>& component : m_components)
		{
			component->AfterLoad();
		}
		for (auto& children : m_children)
		{
			children->AfterLoad();
		}
	}

	List<Weak<GameObject>> GameObject::GetAllChildren() const
	{
		List<Weak<GameObject>> children = GetChildren();
		for (auto& child : m_children)
		{
			List<Weak<GameObject>> newChild = child->GetAllChildren();
			children.insert(children.end(), newChild.begin(), newChild.end());
		}
		return children;
	}

	void SerializeTransform(Utils::Serializer& serializer, Component::Transform* transform)
	{
		serializer << Pair::BEGIN_MAP << "BEGIN TRANSFORM";
		transform->Serialize(serializer);
		serializer << Pair::END_MAP << "END TRANSFORM";
	}

	void SerializeComponent(Utils::Serializer& serializer, const Shared<Component::BaseComponent>& components)
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

		serializer << Pair::KEY << "Name" << Pair::VALUE << m_name;
		serializer << Pair::KEY << "Active" << Pair::VALUE << m_active;
		serializer << Pair::KEY << "UUID" << Pair::VALUE << m_UUID;

		serializer << Pair::KEY << "Component Number" << Pair::VALUE << m_components.size();
		serializer << Pair::KEY << "Child Number" << Pair::VALUE << m_children.size();

		SerializeTransform(serializer, m_transform.get());

		serializer << Pair::BEGIN_TAB;
		for (Shared<Component::BaseComponent>& component : m_components)
		{
			SerializeComponent(serializer, component);
		}
		serializer << Pair::END_TAB;

		serializer << Pair::BEGIN_TAB;
		for (const Shared<GameObject>& child : m_children)
		{
			child->Serialize(serializer);
		}
		serializer << Pair::END_TAB;

		serializer << Pair::END_MAP << "END GAMEOBJECT";
	}

	void GameObject::Deserialize(Utils::Parser& parser, const bool parseUUID /*= true*/)
	{
		m_name = parser["Name"];
		m_active = parser["Active"].As<bool>();
		if (parseUUID)
			m_UUID = parser["UUID"].As<uint64_t>();

		const size_t componentNumber = parser["Component Number"].As<size_t>();
		m_children.resize(parser["Child Number"].As<size_t>());

		parser.NewDepth();
		m_transform->Deserialize(parser);
		m_transform->SetGameObject(shared_from_this());

		for (size_t i = 0; i < componentNumber; i++)
		{
			parser.NewDepth();
			Shared<Component::BaseComponent> component;
			String componentNameString = parser["Name"];
			const bool enable = parser["Enable"].As<bool>();
			const char* componentName = componentNameString.c_str();
			for (const Shared<BaseComponent>& componentInstance : Component::ComponentHolder::GetList())
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

		for (Shared<GameObject>& child : m_children)
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
