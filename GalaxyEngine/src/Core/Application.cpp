
#include "pch.h"

#include "Core/Application.h"
#include "Core/ThreadManager.h"
#include "Core/SceneHolder.h"
#include "Resource/Scene.h"
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

		// Initialize Resource Manager
		m_resourceManager = Resource::ResourceManager::GetInstance();
		m_resourceManager->m_projectPath = projectPath.parent_path();
		m_resourceManager->m_assetPath = projectPath.parent_path() / ASSET_FOLDER_NAME;
		std::string filename = projectPath.filename().string();
		m_resourceManager->m_projectName = filename = filename.substr(0, filename.find_first_of('.'));

		
		m_resourceManager->ImportAllFilesInFolder(m_resourceManager->m_assetPath);
		m_resourceManager->ImportAllFilesInFolder(ENGINE_RESOURCE_FOLDER_NAME);

		
		// Initialize Scene
		m_sceneHolder = Core::SceneHolder::GetInstance();
		
		// Initialize Editor::UI
		m_editorUI = Editor::UI::EditorUIManager::GetInstance();
		m_editorUI->Initialize();

		// Initialize Scripting
		m_scriptEngine = Scripting::ScriptEngine::GetInstance();
		m_scriptEngine->LoadDLL(projectPath.parent_path() / "Generate", filename);

		// Initialize Components
		Component::ComponentHolder::Initialize();
		
		using namespace Utils;

		/*{
			Serializer serializer("tmp.txt");
			serializer << PAIR::BEGIN_MAP << "Test";
			serializer << PAIR::KEY << "path" << PAIR::VALUE << std::filesystem::path("Test/Test.cpp");
			serializer << PAIR::KEY << "float" << PAIR::VALUE << 1.f;
			serializer << PAIR::KEY << "bool" << PAIR::VALUE << false;
			serializer << PAIR::KEY << "int" << PAIR::VALUE << 5;
			serializer << PAIR::KEY << "double" << PAIR::VALUE << 89.65;
			serializer << PAIR::KEY << "Vec2f" << PAIR::VALUE << Vec2f(1.33f, 3.4f);
			serializer << PAIR::KEY << "Vec3f" << PAIR::VALUE << Vec3f(8.5f, 7, 0.1f);
			serializer << PAIR::KEY << "Vec4f" << PAIR::VALUE << Vec4f(8.5f, 7, 0.1f, 12);
			serializer << PAIR::KEY << "Quat" << PAIR::VALUE << Quat(8.5f, 7, 0.1f, 12);

			Weak<Core::GameObject> go = Core::SceneHolder::GetCurrentScene()->CreateObject("Test");
			Weak<Component::MeshComponent> component = go.lock()->AddComponent<Component::MeshComponent>();
			serializer << PAIR::KEY << "Object" << PAIR::VALUE << go.lock().get();
			serializer << PAIR::KEY << "Component" << PAIR::VALUE << dynamic_cast<Component::BaseComponent*>(component.lock().get());
			serializer << PAIR::END_MAP << "Test";
			serializer.CloseFile();

			Parser parser("tmp.txt");
			parser.PrintData();
			auto path = parser["path"];
			auto floatV = parser["float"].As<float>();
			auto intV = parser["int"].As<int>();
			auto doubleV = parser["double"].As<double>();
			auto boolV = parser["bool"].As<bool>();
			auto vector2 = parser["Vec2f"].As<Vec2f>();
			auto vector3 = parser["Vec3f"].As<Vec3f>();
			auto vector4 = parser["Vec4f"].As<Vec4f>();
			auto quat = parser["Quat"].As<Quat>();
			auto object = parser["Object"].As<uint64_t>();
			auto component2 = parser["Component"].As<Component::ComponentID>();
		}*/
	}

	void Core::Application::UpdateResources()
	{
		m_scriptEngine->UpdateFileWatcherDLL();

		if (!m_resourceToSend.empty())
		{
			auto resourcePath = m_resourceToSend.front();
			std::weak_ptr<Resource::IResource> resource = m_resourceManager->GetResource<Resource::IResource>(resourcePath);
			if (resource.expired())
			{
				PrintError("Failed to send resource %s", resourcePath.c_str());
				m_resourceToSend.pop_front();
			}
			else
			{
				if (!resource.lock()->HasBeenSent())
					resource.lock()->Send();
				m_resourceToSend.pop_front();
				if (!resource.lock()->HasBeenSent())
					m_resourceToSend.push_back(resourcePath);
			}
		}
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
