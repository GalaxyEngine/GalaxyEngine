
#include "pch.h"

#include <glad/glad.h>

#include "Core/Application.h"
#include "Core/ThreadManager.h"
#include "Wrapper/GUI.h"
#include "Wrapper/Window.h"
#include "Wrapper/Renderer.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"

#pragma region static
Core::Application Core::Application::m_instance;
#pragma endregion

Core::Application::~Application(){}

void Core::Application::Initalize()
{
	// Initalize Window Lib
	if (!Wrapper::Window::Initialize())
		PrintError("Failed to initalize window API");

	// Create Window
	m_window = std::make_unique<Wrapper::Window>();
	Wrapper::WindowConfig windowConfig;
	windowConfig.width = 800;
	windowConfig.height = 600;
	windowConfig.name = "Galaxy Engine";
	m_window->Create(windowConfig);
	m_window->SetVSync(false);

	// Initalize GUI Lib
	Wrapper::GUI::Initalize(m_window, "#version 330");

	// Initalize Render API
	Wrapper::Renderer::CreateInstance(Wrapper::RenderAPI::OPENGL);
	m_renderer = Wrapper::Renderer::GetInstance();

	m_threadManager = Core::ThreadManager::GetInstance();
	m_threadManager->Initalize();

	m_resourceManager = Resource::ResourceManager::GetInstance();
	m_resourceManager->ImportAllFilesInFolder("Assets");
}

void Core::Application::Update()
{
	Vec4f clear_color = Vec4f(0.45f, 0.55f, 0.60f, 1.00f);
	std::weak_ptr<Resource::Shader> unlitShader = m_resourceManager->GetOrLoad<Resource::Shader>("Assets/unlit.shader");

	while (!m_window->ShouldClose())
	{
		Wrapper::Window::PollEvent();
		Wrapper::GUI::NewFrame();

		if (ImGui::IsKeyPressed(ImGuiKey_F11))
		{
			m_window->ToggleFullscreen();
		}

		//TEMP
		if (!m_resourceToSend.empty())
		{
			auto resourcePath = m_resourceToSend.front();
			std::weak_ptr<Resource::IResource> resource = m_resourceManager->GetResource<Resource::IResource>(resourcePath);
			if (resource.expired()) 
			{
				m_resourceToSend.pop_front();
			}
			else
			{
				resource.lock()->Send();
				m_resourceToSend.pop_front();
				if (!resource.lock()->HasBeenSent())
					m_resourceToSend.push_back(resourcePath);
			}
		}

		//BEGINDRAW
		if (ImGui::Begin("Window"))
		{
			int i = 0;
			for (std::weak_ptr<Resource::Texture> texture : m_resourceManager->GetAllResources<Resource::Texture>())
			{
				i++;
				void* id = reinterpret_cast<void*>(static_cast<uintptr_t>(texture.lock()->GetID()));
				ImGui::ImageButton(id, Vec2f(128, 128));
				if (i % 3 != 0)
					ImGui::SameLine();
			}
			ImGui::Text("%f", 1.f / ImGui::GetIO().DeltaTime);
		}
		ImGui::End();
		//ENDDRAW

		// Rendering
		m_renderer->ClearColorAndBuffer(clear_color);

		Wrapper::GUI::EndFrame(m_window);

		m_window->SwapBuffers();
	}
}

void Core::Application::Destroy()
{
	// Cleanup:
	// Thread Manager
	m_threadManager->Destroy();

	// GUI
	Wrapper::GUI::UnInitalize();

	// Window
	m_window->Destroy();
	Wrapper::Window::UnInitialize();

	PrintLog("Application clean-up completed.");
}

void GALAXY::Core::Application::AddResourceToSend(const std::string& fullPath)
{
	if (!std::count(m_resourceToSend.begin(), m_resourceToSend.end(), fullPath))
		m_resourceToSend.push_back(fullPath);
}
