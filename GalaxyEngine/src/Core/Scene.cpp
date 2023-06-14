#include "pch.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "EditorUI/EditorUIManager.h"

using namespace Core;
Scene::Scene()
{
	m_root = std::make_shared<GameObject>("Scene");
	m_root->AddChild(CreateObject("Child 0"));
	m_root->AddChild(CreateObject("Child 1"));
}

Scene::~Scene()
{
}

void Scene::Update()
{
	EditorUI::EditorUIManager::GetInstance()->DrawUI();

	m_root->UpdateSelfAndChild();

}

std::weak_ptr<GameObject> Scene::GetRootGameObject() const
{
	return m_root;
}

uint64_t Scene::GetFreeIndex()
{
	uint64_t index = 0;
	while (m_objectList.find(index) != m_objectList.end()) {
		index++;
	}
	return index;
}

void Scene::RemoveObject(GameObject* object)
{
	auto shared = std::find_if(m_objectList.begin(), m_objectList.end(), [&](const std::pair<uint64_t, std::shared_ptr<GameObject>>& element) {
		return element.second.get() == object; });
	if (shared != m_objectList.end())
	{
		m_objectList.erase(shared);
	}
}

std::weak_ptr<GameObject> Scene::GetWithIndex(uint64_t index)
{
	if (m_objectList.count(index))
	{
		return m_objectList.at(index);
	}
	return std::weak_ptr<GameObject>();
}
