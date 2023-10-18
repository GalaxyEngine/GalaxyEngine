#include "pch.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"

std::unique_ptr<Core::SceneHolder> Core::SceneHolder::m_instance;

Core::SceneHolder::~SceneHolder()
{
}

Core::SceneHolder* Core::SceneHolder::GetInstance()
{
	if (!m_instance)
	{
		m_instance = std::make_unique<Core::SceneHolder>();
		m_instance->m_currentScene = std::make_unique<Resource::Scene>("Scene");
	}
	return m_instance.get();
}

void Core::SceneHolder::Update()
{
	m_currentScene->Update();
	SwitchSceneUpdate();
}

Resource::Scene* Core::SceneHolder::GetCurrentScene()
{
	return GetInstance()->m_currentScene.get();
}

void Core::SceneHolder::Release()
{
	m_currentScene.reset();
	m_instance.reset();
}

void Core::SceneHolder::SwitchScene(Weak<Resource::Scene> scene)
{
	m_nextScene = scene.lock();
}

void Core::SceneHolder::SwitchSceneUpdate()
{
	if (m_nextScene)
	{
		m_currentScene = m_nextScene;
		m_nextScene.reset();
	}
}
