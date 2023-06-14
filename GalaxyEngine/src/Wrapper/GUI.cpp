#include "pch.h"
#include "Wrapper/GUI.h"
#include "Wrapper/Window.h"

void Wrapper::GUI::Initalize(const std::unique_ptr<Wrapper::Window>& window, const char* glsl_version)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	GLFWwindow* windowPtr = static_cast<GLFWwindow*>(window->GetWindow());
	ImGui_ImplGlfw_InitForOpenGL(windowPtr, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	
	PrintLog("Initalized ImGui %s", IMGUI_VERSION);
}

void Wrapper::GUI::UnInitalize()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Wrapper::GUI::NewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Wrapper::GUI::EndFrame(const std::unique_ptr<Wrapper::Window>& window)
{
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = static_cast<GLFWwindow*>(window->GetCurrentContext());
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		Wrapper::Window::MakeContextCurrent(backup_current_context);
	}
}

float Wrapper::GUI::DeltaTime()
{
	return ImGui::GetIO().DeltaTime;
}