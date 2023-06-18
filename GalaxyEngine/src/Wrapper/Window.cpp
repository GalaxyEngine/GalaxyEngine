#include "pch.h"
#include "Wrapper/Window.h"
#include <GLFW/glfw3.h>

Wrapper::Window::~Window()
{
}

static void glfw_error_callback(int error, const char* description)
{
	PrintError("GLFW Error %d: %s", error, description);
}

bool Wrapper::Window::Initialize()
{
	if (!glfwInit()) {
		PrintError("Failed to initalize GLFW");
		return false;
	}
	PrintLog("Initalized GLFW %d.%d", 3, 0);
	glfwSetErrorCallback(glfw_error_callback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	return true;
}

void Wrapper::Window::UnInitialize()
{
	glfwTerminate();
}

void Wrapper::Window::PollEvent()
{
	glfwPollEvents();
}

void* Wrapper::Window::GetCurrentContext()
{
	return glfwGetCurrentContext();
}

void Wrapper::Window::MakeContextCurrent(void* window)
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window);
	glfwMakeContextCurrent(glfwWindow);
}

void Wrapper::Window::Create(const WindowConfig& config)
{
	if (m_window = glfwCreateWindow(config.width, config.height, config.name, nullptr, nullptr); !m_window)
		PrintError("Failed to create window");
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwMakeContextCurrent(glfwWindow);
	glfwSetFramebufferSizeCallback(glfwWindow, &ResizeCallback);
}

void Wrapper::Window::Destroy()
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwDestroyWindow(glfwWindow);
}

void Wrapper::Window::SwapBuffers()
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwSwapBuffers(glfwWindow);
}

bool Wrapper::Window::GetVSyncEnable()
{
	return m_vsync;
}


void Wrapper::Window::GetSize(int* width, int* height)
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwGetWindowSize(glfwWindow, width, height);
}

Vec2i Wrapper::Window::GetSize()
{
	Vec2i size;
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwGetWindowSize(glfwWindow, &size.x, &size.y);
	return size;
}

void Wrapper::Window::SetVSync(bool enable)
{
	m_vsync = enable;
	glfwSwapInterval(enable); // Enable vsync
}

bool Wrapper::Window::ShouldClose()
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	return glfwWindowShouldClose(glfwWindow);
}

void Wrapper::Window::SetSize(int width, int height)
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwSetWindowSize(glfwWindow, width, height);
}

void Wrapper::Window::SetSize(const Vec2i& size)
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwSetWindowSize(glfwWindow, size.x, size.y);
}

void* Wrapper::Window::GetProcAddress(const char* procname)
{
	return reinterpret_cast<void*>(glfwGetProcAddress(procname));
}

void Wrapper::Window::ToggleFullscreen()
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	SetFullscreen(glfwGetWindowMonitor(glfwWindow) == nullptr);
}

void Wrapper::Window::SetFullscreen(bool enable)
{
	static Vec2i prevSize;
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	if (enable) {
		prevSize = GetSize();
		GLFWmonitor* primary = glfwGetPrimaryMonitor();

		const GLFWvidmode* mode = glfwGetVideoMode(primary);

		glfwSetWindowMonitor(glfwWindow, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
		SetVSync(m_vsync);
	}
	else
	{
		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primary);
		glfwSetWindowMonitor(glfwWindow, NULL, 0, 0, (int)prevSize.x, (int)prevSize.y, 0);
		glfwSetWindowPos(glfwWindow, 100, 100);
	}
}

void Wrapper::Window::ResizeCallback(GLFWwindow* window, int width, int height)
{
	Renderer* renderer = Wrapper::Renderer::GetInstance();
	if (renderer && renderer->IsInitalized())
	{
		renderer->Viewport({ 0, 0 }, { width, height });
	}
}
