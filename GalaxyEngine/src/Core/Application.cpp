
#include "pch.h"

#include "Core/Application.h"
#include "Core/ThreadManager.h"
#include "Core/SceneHolder.h"
#include "Core/Input.h"
#include "Core/GameObject.h"

#include "Wrapper/GUI.h"
#include "Wrapper/Window.h"
#include "Wrapper/Renderer.h"
#include "Wrapper/Audio.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#include "Render/LightManager.h"

#ifdef WITH_EDITOR
#include "Editor/UI/EditorUIManager.h"
#include "Editor/ThumbnailCreator.h"
#endif

#include "Component/ComponentHolder.h"

#include "Resource/IResource.h"

#include "Scripting/ScriptEngine.h"

#include "Utils/FileInfo.h"
#include "Utils/Time.h"

#include <cpp_serializer/CppSerializer.h>

#include "Render/Skybox.h"
#include "Wrapper/PhysicsWrapper.h"

#include "Utils/OS.h"

namespace GALAXY {
#pragma region static
	Core::Application Core::Application::m_instance;
#pragma endregion

	void Core::Application::Initialize(std::filesystem::path projectPath)
	{
		Debug::Log::LogToFile = true;
		
		// Create folder that not exist
#ifdef WITH_EDITOR
		if (!std::filesystem::exists(THUMBNAIL_PATH))
			std::filesystem::create_directories(THUMBNAIL_PATH);
#endif
		const auto logPath = Utils::OS::GetEngineDataFolder() / LOG_PATH;
		if (!std::filesystem::exists(logPath))
			std::filesystem::create_directories(logPath);
		
#ifdef WITH_EDITOR
		m_editorSettings.LoadSettings();
		if (projectPath.empty())
		{
			projectPath = m_editorSettings.GetDefaultProjectPath();
		}
#endif

		std::cout << projectPath << '\n';
		// Initialize Window Lib
		if (!Wrapper::Window::Initialize())
			PrintError("Failed to initialize window API");

		// Create Window
		m_window = std::make_unique<Wrapper::Window>();
		Wrapper::WindowConfig windowConfig;
		windowConfig.width = 1600;
		windowConfig.height = 900;
#ifdef WITH_EDITOR
		windowConfig.name = "Galaxy Engine";
#else
		std::string projectName = projectPath.filename().stem().string();
		windowConfig.name = projectName.c_str();
#endif
		m_window->Create(windowConfig);
#ifdef WITH_EDITOR
		m_window->SetVSync(m_editorSettings.GetShouldUseVSync());
		m_window->SetIcon(ENGINE_RESOURCE_FOLDER_NAME"/icons/logo_256.png");
#else
		m_window->SetVSync(true);
#endif

		Wrapper::PhysicsWrapper::Initialize(Wrapper::PhysicAPIType::Jolt);
		m_physicsWrapper = Wrapper::PhysicsWrapper::GetInstance();

		m_audioSystem = Wrapper::Audio::GetInstance();
		m_audioSystem->Initialize();

		// Initialize GUI Lib
		Wrapper::GUI::Initialize(m_window, "#version 450");

		// Initialize Render API
		Wrapper::Renderer::CreateInstance(Wrapper::RenderAPI::OPENGL);
		m_renderer = Wrapper::Renderer::GetInstance();
		m_window->SetSize(m_window->GetSize() * m_window->GetScreenScale());

		// Initialize Thread Manager
		m_threadManager = Core::ThreadManager::GetInstance();
		m_threadManager->Initialize();

#ifdef WITH_EDITOR
		m_editorUI = Editor::UI::EditorUIManager::CreateInstance();

		// Create Thumbnail Creator before ResourceManager
		m_thumbnailCreator = new Editor::ThumbnailCreator();
#endif

		// Initialize Resource Manager
		m_resourceManager = Resource::ResourceManager::GetInstance();
		m_resourceManager->m_projectExists = std::filesystem::exists(projectPath);
		m_resourceManager->m_projectPath = projectPath.parent_path();
		m_resourceManager->m_assetPath = projectPath.parent_path() / ASSET_FOLDER_NAME;
		std::string filename = projectPath.filename().generic_string();
		m_resourceManager->m_projectName = filename = filename.substr(0, filename.find_first_of('.'));

		// Initialize Scripting
		m_scriptEngine = Scripting::ScriptEngine::GetInstance();

		/* Import all resources and parse UUID
		*  Load all resources that need to be load but after import because of reference resources
		*/
		m_resourceManager->ImportAllFilesInFolder(m_resourceManager->m_assetPath);
		m_resourceManager->ImportAllFilesInFolder(ENGINE_RESOURCE_FOLDER_NAME);
		Render::Skybox::Initialize();
		m_resourceManager->LoadNeededResources();
		m_resourceManager->ReadCache();
		m_projectSettings.LoadSettings();

		// Initialize Components
		Component::ComponentHolder::Initialize();
		m_scriptEngine->RegisterScriptComponents();
		
		// Initialize Scene
		m_sceneHolder = Core::SceneHolder::GetInstance();
		
#ifdef WITH_EDITOR
		// Initialize Editor::UI
		m_editorUI->Initialize();

		m_editorSettings.LoadThumbnail();
#else
		m_window->SetIcon(Resource::ResourceManager::GetProjectPath() / m_projectSettings.GetProjectIconPath());
#endif

		// Load dll scripting
		if (m_resourceManager->m_projectExists)
		{
			const std::filesystem::path dllPath = projectPath.parent_path() / "Generate" / m_resourceManager->m_projectName;
			m_scriptEngine->LoadDLL(dllPath.generic_string().c_str());
		}
	}

	void Core::Application::UpdateResources()
	{
#ifdef WITH_EDITOR
		m_scriptEngine->UpdateFileWatch();
		m_thumbnailCreator->Update();
#endif

		if (!m_resourceToSend.empty())
		{
			const Path resourcePath = m_resourceToSend.front();
			const std::weak_ptr<Resource::IResource> resource = Resource::ResourceManager::GetResource<Resource::IResource>(resourcePath);

			if (const Shared<Resource::IResource> resourceFound = resource.lock())
			{
				TrySendResource(resourceFound, resourcePath);
			}
			else if (const Shared<Resource::IResource> temporaryResourceFound = m_resourceManager->GetTemporaryResource<Resource::IResource>(resourcePath))
			{
				PrintLog("Send temp resource %s", resourcePath.string().c_str());
				TrySendResource(temporaryResourceFound, resourcePath);
			}
			else
			{
				PrintError("Failed to send resource %s", resourcePath.string().c_str());
				m_resourceToSend.pop_front();
			}
		}
	}

	void Core::Application::TrySendResource(const Shared<Resource::IResource>& resource, const std::filesystem::path& resourcePath)
	{
		if (!resource->HasBeenSent())
			resource->Send();
		m_resourceToSend.pop_front();
		if (!resource->HasBeenSent())
			m_resourceToSend.push_back(resourcePath);
	}

	void Core::Application::Update()
	{
		while (!m_window->ShouldClose())
		{
			Utils::Time::UpdateDeltaTime();

#ifdef WITH_EDITOR
			m_editorUI->UpdateDPIScale();
#endif

			Wrapper::Window::PollEvent();
			Wrapper::GUI::NewFrame();

			if (Input::IsKeyPressed(Key::F11))
			{
				m_window->ToggleFullscreen();
			}
#ifdef WITH_EDITOR
			if (Input::IsKeyPressed(Key::F5))
			{
				m_resourceManager->GetUnlitShader().lock()->Recompile();
			}
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_C))
			{
				CopyObject();
			}
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_V))
			{
				PasteObject();
			}
			if (ImGui::IsKeyDown(ImGuiKey_F1))
			{
				while (true)
				{

				}
			}
#endif

			UpdateResources();

			//BEGINDRAW
			m_sceneHolder->Update();
			//ENDDRAW

#ifdef WITH_EDITOR
			m_editorSettings.UpdateScreenShot();
#endif

			// Rendering
			Wrapper::GUI::EndFrame(m_window);

			// Update Inputs
			Input::Update();

			m_window->SwapBuffers();

#ifdef WITH_EDITOR
			m_benchmark.UpdateBenchmark(Utils::Time::DeltaTime());
#endif
		}
	}

#ifdef WITH_EDITOR
	void Core::Application::PasteObject() const
	{
		if (m_clipboard.empty())
			return;
		auto parser = CppSer::Parser(m_clipboard);
		const List<Weak<GameObject>> selected = m_editorUI->GetInspector()->GetSelectedGameObjects();

		Shared<GameObject> parent;
		if (selected.empty())
		{
			parent = SceneHolder::GetCurrentScene()->GetRootGameObject().lock();
		}
		else
		{
			parent = selected[0].lock()->GetParent();
			if (!parent)
				parent = selected[0].lock();
		}

		m_editorUI->GetInspector()->ClearSelected();
		// Parse all gameObject
		do
		{
			auto object = SceneHolder::GetCurrentScene()->CreateObject().lock();

			object->SetParent(parent);

			object->Deserialize(parser, false);

			m_editorUI->GetInspector()->AddSelected(object);

			parser.PushDepth();
		} while (parser.GetValueMap().size() != parser.GetCurrentDepth());
	}

	void Core::Application::CopyObject()
	{
		const List<Weak<GameObject>>& selected = m_editorUI->GetInspector()->GetSelectedGameObjects();

		List<Weak<GameObject>> objects;

		for (const Weak<GameObject>& object : selected)
		{
			bool canBeAdded = true;
			Shared<GameObject> lockObject = object.lock();
			for (const Weak<GameObject>& object2 : selected)
			{
				Shared<GameObject> lockObject2 = object2.lock();
				if (lockObject != lockObject2 && lockObject->IsAParent(lockObject2.get()))
				{
					canBeAdded = false;
				}
			}
			if (canBeAdded)
			{
				objects.push_back(object);
			}
		}

		std::ranges::sort(objects, [](const Weak<GameObject>& a, const Weak<GameObject>& b)
			{
				return a.lock()->GetSceneGraphID() < b.lock()->GetSceneGraphID();
			});

		auto serializer = CppSer::Serializer();
		// Temporary : TODO : move this directly int the canBeAdded if bracket
		for (Weak<GameObject>& object : objects)
		{
			// Serialize all GameObject in same content
			object.lock()->Serialize(serializer);
		}
		m_clipboard = serializer.GetContent();
	}

	void Core::Application::SetApplicationMode(Editor::ApplicationMode mode)
	{
		if (m_applicationMode == mode)
		{
			PrintError("Application mode already set to %s", m_applicationMode == Editor::ApplicationMode::Editor ? "Editor" : "Play");
			return;
		}

		auto projectPath = Resource::ResourceManager::GetProjectPath();
		if (mode == Editor::ApplicationMode::Play && m_applicationMode == Editor::ApplicationMode::Editor)
		{
			Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
			currentScene->Save(projectPath / PLAYMODE_SCENE_PATH);

			currentScene->GetRootGameObject().lock()->StartSelfAndChild();
			
		}
		else if (m_applicationMode == Editor::ApplicationMode::Editor && mode == Editor::ApplicationMode::Play)
		{
			const auto sceneResource = Resource::ResourceManager::ReloadResource<Resource::Scene>(projectPath / PLAYMODE_SCENE_PATH);

			// Only set the date of the scene			
			Core::SceneHolder::GetInstance()->SwitchScene(sceneResource, true);
		}
		m_applicationMode = mode;
	}
#endif

	void Core::Application::Destroy() const
	{
#ifdef WITH_EDITOR
		m_resourceManager->CreateCache();
		
		// Cleanup:
		if (m_thumbnailCreator) {
			m_thumbnailCreator->Release();
			delete m_thumbnailCreator;
		}

		if (m_editorUI)
			m_editorUI->Release();
#endif

		if (m_sceneHolder)
			m_sceneHolder->Release();

		// Thread Manager
		if (m_threadManager)
			m_threadManager->Destroy();

		// Resource Manager
		if (m_resourceManager)
			m_resourceManager->Release();

		Component::ComponentHolder::Release();

		// GUI
		Wrapper::GUI::UnInitalize();

		// Audio Wrapper
		if (m_audioSystem)
			m_audioSystem->Release();

		// Physics Wrapper
		if (m_physicsWrapper)
			m_physicsWrapper->Release();

		// Window
		if (m_window)
			m_window->Destroy();

		Wrapper::Window::UnInitialize();

		PrintLog("Application clean-up completed.");

		Debug::Log::CloseFile();
	}

	void Core::Application::Exit() const
	{
		m_window->Close();
	}
}
