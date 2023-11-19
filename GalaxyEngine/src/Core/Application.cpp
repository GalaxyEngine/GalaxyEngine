
#include "pch.h"

#include "Core/Application.h"
#include "Core/ThreadManager.h"
#include "Core/SceneHolder.h"
#include "Core/Input.h"
#include "Core/GameObject.h"

#include "Wrapper/GUI.h"
#include "Wrapper/Window.h"
#include "Wrapper/Renderer.h"
#include "Wrapper/ImageLoader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Scene.h"

#include "Render/LightManager.h"

#include "Editor/UI/EditorUIManager.h"

#include "Component/ComponentHolder.h"
#include "Component/MeshComponent.h"

#include "Scripting/ScriptEngine.h"

#include "Resource/IResource.h"

#include "Utils/FileInfo.h"
#include "Utils/Parser.h"

namespace GALAXY {
#pragma region static
	Core::Application Core::Application::m_instance;
#pragma endregion

	Core::Application::~Application() {}

	void Core::Application::Initialize(const std::filesystem::path& projectPath)
	{
		// Initialize Window Lib
		if (!Wrapper::Window::Initialize())
			PrintError("Failed to initialize window API");

		// Create Window
		m_window = std::make_unique<Wrapper::Window>();
		Wrapper::WindowConfig windowConfig;
		windowConfig.width = 1600;
		windowConfig.height = 900;
		windowConfig.name = "Galaxy Engine";
		m_window->Create(windowConfig);
		m_window->SetVSync(false);

		Input::Initialize();

		// Initialize GUI Lib
		Wrapper::GUI::Initialize(m_window, "#version 450");

		// Initialize Render API
		Wrapper::Renderer::CreateInstance(Wrapper::RenderAPI::OPENGL);
		m_renderer = Wrapper::Renderer::GetInstance();

		// Initialize Thread Manager
		m_threadManager = Core::ThreadManager::GetInstance();
		m_threadManager->Initialize();

		// Initialize Light Manager
		m_lightManager = Render::LightManager::GetInstance();

		// Initialize Resource Manager
		m_resourceManager = Resource::ResourceManager::GetInstance();
		m_resourceManager->m_projectPath = projectPath.parent_path();
		m_resourceManager->m_assetPath = projectPath.parent_path() / ASSET_FOLDER_NAME;
		std::string filename = projectPath.filename().generic_string();
		m_resourceManager->m_projectName = filename = filename.substr(0, filename.find_first_of('.'));

		m_resourceManager->ImportAllFilesInFolder(m_resourceManager->m_assetPath);
		m_resourceManager->ImportAllFilesInFolder(ENGINE_RESOURCE_FOLDER_NAME);
		
		// Initialize Scene
		m_sceneHolder = Core::SceneHolder::GetInstance();
		
		// Initialize Editor::UI
		Editor::UI::EditorUIManager::Initialize();
		m_editorUI = Editor::UI::EditorUIManager::GetInstance();

		// Initialize Scripting
		m_scriptEngine = Scripting::ScriptEngine::GetInstance();
		m_scriptEngine->LoadDLL(projectPath.parent_path() / "Generate", filename);

		// Initialize Components
		Component::ComponentHolder::Initialize();
	}

	void Core::Application::UpdateResources()
	{
		m_scriptEngine->UpdateFileWatcherDLL();

		if (!m_resourceToSend.empty())
		{
			auto resourcePath = m_resourceToSend.front();
			std::weak_ptr<Resource::IResource> resource = m_resourceManager->GetResource<Resource::IResource>(resourcePath);

			if (auto resourceFound = resource.lock())
			{
				TrySendResource(resourceFound, resourcePath);
			}
			else if (auto resourceFound = m_resourceManager->GetTemporaryResource<Resource::IResource>(resourcePath))
			{
				PrintLog("Send temp resource %s", resourcePath.string().c_str());
				TrySendResource(resourceFound, resourcePath);
			}
			else
			{
				PrintError("Failed to send resource %s", resourcePath.string().c_str());
				m_resourceToSend.pop_front();
			}
		}
	}

	void Core::Application::TrySendResource(Shared<Resource::IResource> resource, const std::filesystem::path& resourcePath)
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
			Wrapper::Window::PollEvent();
			Wrapper::GUI::NewFrame();

			if (Input::IsKeyPressed(Key::F11))
			{
				m_window->ToggleFullscreen();
			}
			if (Input::IsKeyPressed(Key::F5))
			{
				m_resourceManager->GetLitShader().lock()->Recompile();
			}
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_C))
			{
				CopyObject();
			}
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_V))
			{
				PasteObject();
			}

			UpdateResources();

			//BEGINDRAW
			m_sceneHolder->Update();
			//ENDDRAW

			// Rendering
			Wrapper::GUI::EndFrame(m_window);

			// Update Inputs
			Input::Update();

			m_window->SwapBuffers();
		}
	}

	void Core::Application::PasteObject()
	{
		auto parser = Utils::Parser(m_clipboard);
		List<Weak<GameObject>> selected = m_editorUI->GetInspector()->GetSelected();

		if (selected.empty())
			return;

		Shared<GameObject> parent = selected[0].lock()->GetParent();
		if (!parent)
			parent = selected[0].lock();

		m_editorUI->GetInspector()->ClearSelected();
		// Parse all gameObject
		do
		{
			auto object = SceneHolder::GetCurrentScene()->CreateObject().lock();

			object->SetParent(parent);

			object->Deserialize(parser, false);

			m_editorUI->GetInspector()->AddSelected(object);

			parser.NewDepth();
		} while (parser.GetValueMap().size() != parser.GetCurrentDepth());
	}

	void Core::Application::CopyObject()
	{
		List<Weak<GameObject>> selected = m_editorUI->GetInspector()->GetSelected();

		List<Weak<GameObject>> objects;

		for (auto& object : selected)
		{
			bool canBeAdded = true;
			auto lockObject = object.lock();
			for (auto& object2 : selected)
			{
				auto lockObject2 = object2.lock();
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

		std::sort(objects.begin(), objects.end(), [](const Weak<GameObject>& a, const Weak<GameObject>& b)
			{
				return a.lock()->GetSceneGraphID() < b.lock()->GetSceneGraphID();
			});

		auto serializer = Utils::Serializer();
		// Temporary : TODO : move this directly int the canBeAdded if bracket
		for (auto& object : objects)
		{
			// Serialize all GameObject in same content
			object.lock()->Serialize(serializer);
		}
		m_clipboard = serializer.GetContent();
	}

	void Core::Application::Destroy()
	{
		// Cleanup:
		// Scene Holder
		if (m_editorUI)
			m_editorUI->Release();

		if (m_sceneHolder)
			m_sceneHolder->Release();

		// Thread Manager
		if (m_threadManager)
			m_threadManager->Destroy();

		// Resource Manager
		if (m_resourceManager)
			m_resourceManager->Release();

		// GUI
		Wrapper::GUI::UnInitalize();

		// Window
		if (m_window)
			m_window->Destroy();

		Wrapper::Window::UnInitialize();

		PrintLog("Application clean-up completed.");
	}

	void Core::Application::Exit()
	{
		m_window->Close();
	}
}
