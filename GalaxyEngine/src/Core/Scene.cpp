#include "pch.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"

std::unique_ptr<Core::Scene> Core::Scene::m_instance;

Core::Scene::Scene()
{
	m_root = std::make_shared<class GameObject>("Scene");
}

Core::Scene::~Scene()
{
}

Core::Scene* Core::Scene::GetInstance()
{
	if (!m_instance)
	{
		m_instance = std::make_unique<Scene>();
	}
	return m_instance.get();
}

void Core::Scene::Update()
{

}
