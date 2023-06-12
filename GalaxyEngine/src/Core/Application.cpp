
#include "pch.h"

#include <glad/glad.h>

#include "Core\Application.h"
#include "Wrapper/GUI.h"
#include "Wrapper/Window.h"
#include "Wrapper/Renderer.h"
#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

#pragma region static
Core::Application Core::Application::m_instance;
#pragma endregion

Core::Application::~Application()
{
}

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
	m_window->SetVSync(true);

	// Initalize GUI Lib
	Wrapper::GUI::Initalize(m_window, "#version 130");

	// Initalize Render API
	Wrapper::Renderer::CreateInstance(Wrapper::RenderAPI::OPENGL);

	m_resourceManager = Resource::ResourceManager::GetInstance();
	m_resourceManager->ImportAllFilesInFolder("Assets");
}

void Core::Application::Update()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	std::weak_ptr<Resource::Texture> texture = m_resourceManager->GetOrLoad<Resource::Texture>("Assets/debug_texture.png");

	while (!m_window->ShouldClose())
	{
		Wrapper::Window::PollEvent();
		Wrapper::GUI::NewFrame();

		//BEGINDRAW
		if (ImGui::Begin("Window"))
		{
			ImGui::Text("%f", 1.f / ImGui::GetIO().DeltaTime);
			void* id = reinterpret_cast<void*>(static_cast<uintptr_t>(texture.lock()->GetID()));
			ImGui::Image(id, ImVec2(128, 128));
		}
		ImGui::End();
		//ENDDRAW

		// Rendering
		int display_w, display_h;
		m_window->GetSize(&display_w, &display_h);

		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		Wrapper::GUI::EndFrame(m_window);

		m_window->SwapBuffers();
	}

}

void Core::Application::Destroy()
{
	// Cleanup:

	// GUI
	Wrapper::GUI::UnInitalize();

	// Window
	m_window->Destroy();
	Wrapper::Window::UnInitialize();

	PrintLog("Application clean-up completed.");
}
