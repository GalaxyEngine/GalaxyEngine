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

void Core::SceneHolder::OpenScene(const std::filesystem::path& path)
{
#ifdef WITH_EDITOR
	if (m_instance->first && Core::Application::IsEditorMode() && Editor::UI::EditorUIManager::ShouldDisplaySafeClose())
	{
		auto onValidateEvent = [&, path]() { OpenScene(path); };
		Editor::UI::EditorUIManager::GetInstance()->SetOnValidatePopupEvent(onValidateEvent);
		m_instance->first = false;
		return;
	}
	else
	{
		Weak<Resource::Scene> sceneResource;
		if (Core::SceneHolder::GetCurrentScene() != Resource::ResourceManager::GetResource<Resource::Scene>(path).lock().get())
		{
			sceneResource = Resource::ResourceManager::ReloadResource<Resource::Scene>(path);
		}
		else
		{
			// if the scene is the same as the current, reload it only in the SwitchUpdate() with async = false
			sceneResource = Resource::ResourceManager::GetResource<Resource::Scene>(path);
		}

		m_instance->SwitchScene(sceneResource);
		m_instance->first = true;
	}
	
#else
	Weak<Resource::Scene> sceneResource;
	if (Core::SceneHolder::GetCurrentScene() != Resource::ResourceManager::GetResource<Resource::Scene>(path).lock().get())
	{
		sceneResource = Resource::ResourceManager::ReloadResource<Resource::Scene>(path);
	}
	else
	{
		// if the scene is the same as the current, reload it only in the SwitchUpdate() with async = false
		sceneResource = Resource::ResourceManager::GetResource<Resource::Scene>(path);
	}

	m_instance->SwitchScene(sceneResource);
#endif
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
#ifdef WITH_EDITOR
	if (m_nextScene && m_nextScene->IsLoaded() && !m_copyData)
	{
		Editor::UI::EditorUIManager::GetInstance()->GetInspector()->ClearSelected();
		m_nextScene->m_editorCamera = m_currentScene->m_editorCamera;
		if (m_currentScene != m_nextScene) {
			// Do not unload if the next scene is the same as the current scene
			Resource::ResourceManager::GetInstance()->RemoveResource(m_currentScene);
		}
		else
		{
			auto editorCamera = m_currentScene->m_editorCamera;
			Resource::ResourceManager::ReloadResource<Resource::Scene>(m_currentScene->GetFileInfo().GetFullPath(), false);
			m_nextScene->Send();
			m_nextScene->m_editorCamera = editorCamera;
		}
		m_currentScene.reset();

		m_currentScene = m_nextScene;
		m_nextScene.reset();
	}
	else if (m_nextScene && m_nextScene->IsLoaded() && m_copyData)
	{
		m_currentScene->SetData(m_nextScene.get());	
		m_nextScene.reset();
	}
#else
	if (m_nextScene && m_nextScene->IsLoaded())
	{
		if (m_currentScene != m_nextScene) {
			// Do not unload if the next scene is the same as the current scene
			Resource::ResourceManager::GetInstance()->RemoveResource(m_currentScene);
		}
		m_currentScene.reset();

		m_currentScene = m_nextScene;
		m_nextScene.reset();
	}
#endif
}
