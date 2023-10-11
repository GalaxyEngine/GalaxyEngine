#include "pch.h"
#include "Core/SceneHolder.h"
#include "Core/Scene.h"

std::unique_ptr<Core::SceneHolder> Core::SceneHolder::m_instance;

Core::SceneHolder::~SceneHolder()
{
}

Core::SceneHolder* Core::SceneHolder::GetInstance()
{
	if (!m_instance)
	{
		m_instance = std::make_unique<Core::SceneHolder>();
	}
	return m_instance.get();
}

void Core::SceneHolder::Update()
{
	if (m_currentScene)
	{
		m_currentScene->Update();
	}
	else
	{
		// TODO : Temp
		m_currentScene = std::make_shared<Scene>();
	}
}

Core::Scene* Core::SceneHolder::GetCurrentScene()
{
	return GetInstance()->m_currentScene.get();
}

void Core::SceneHolder::Release()
{
	m_currentScene.reset();
	m_instance.reset();
}
