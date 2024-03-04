#include "pch.h"
#include "Core/SceneHolder.h"
#include "Core/Application.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#ifdef WITH_EDITOR
#include "Editor/UI/EditorUIManager.h"
#include "Editor/UI/Inspector.h"
#include "Editor/EditorCamera.h"
#endif

std::unique_ptr<Core::SceneHolder> Core::SceneHolder::m_instance;

Core::SceneHolder::~SceneHolder()
{
}

Core::SceneHolder* Core::SceneHolder::GetInstance()
{
	if (!m_instance)
	{
		m_instance = std::make_unique<Core::SceneHolder>();
#ifdef WITH_EDITOR
		m_instance->m_currentScene = std::make_unique<Resource::Scene>("Scene");
		m_instance->m_currentScene->m_editorCamera = std::make_unique<Render::EditorCamera>();
		m_instance->m_currentScene->Initialize();
#else
		auto scenePath = Core::Application::GetInstance().GetProjectSettings().GetStartScene();
		auto scene = Resource::ResourceManager::GetOrLoad<Resource::Scene>(scenePath);
		m_instance->SwitchScene(scene);
#endif
	}
	return m_instance.get();
}

void Core::SceneHolder::Update()
{
	if (m_currentScene)
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

void Core::SceneHolder::SwitchSceneUpdate()
{
	if (m_nextScene && m_nextScene->IsLoaded())
	{
#ifdef WITH_EDITOR
		Editor::UI::EditorUIManager::GetInstance()->GetInspector()->ClearSelected();
		m_nextScene->m_editorCamera = m_currentScene->m_editorCamera;
#endif
		if (m_currentScene != m_nextScene) {
			// Do not unload if the next scene is the same as the current scene
			Resource::ResourceManager::GetInstance()->RemoveResource(m_currentScene);
		}
		m_currentScene.reset();

		m_currentScene = m_nextScene;
		m_nextScene.reset();
	}
}
