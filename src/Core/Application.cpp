#include "Core\Application.h"

#include <GL/glew.h>

#include <Wrapper/GUI.h>
#include <Wrapper/Window.h>
Core::Application Core::Application::m_instance;

Core::Application::~Application()
{
}

void Core::Application::Update()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!m_window->ShouldClose())
	{
		Wrapper::Window::PollEvent();
		Wrapper::GUI::NewFrame();

		//BEGINDRAW
		if (ImGui::Begin("Window"))
		{
			ImGui::Text("%f", 1.f / ImGui::GetIO().DeltaTime);
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
}

void Core::Application::Initalize()
{
	// Initalize Window Lib
	Wrapper::Window::Initialize();

	// Create Window
	m_window = std::make_unique<Wrapper::Window>();
	Wrapper::WindowConfig windowConfig;
	windowConfig.width = 800;
	windowConfig.height = 600;
	windowConfig.name = "Test Window";
	m_window->Create(windowConfig);
	m_window->SetVSync(true);

	// Initalize GUI Lib
	Wrapper::GUI::Initalize(m_window, "#version 130");

}
