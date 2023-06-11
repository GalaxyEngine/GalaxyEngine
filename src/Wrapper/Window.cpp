#include "pch.h"
#include "Wrapper/Window.h"
#include <glfw/glfw3.h>

Wrapper::Window::~Window()
{
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool Wrapper::Window::Initialize()
{
	if (!glfwInit())
		return false;
	glfwSetErrorCallback(glfw_error_callback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
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
		throw "failed to Create Window";
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwMakeContextCurrent(glfwWindow);
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

void Wrapper::Window::GetSize(int* width, int* height)
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	glfwGetWindowSize(glfwWindow, width, height);
}

void Wrapper::Window::SetVSync(bool enable)
{
	glfwSwapInterval(enable); // Enable vsync
}

bool Wrapper::Window::ShouldClose()
{
	GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
	return glfwWindowShouldClose(glfwWindow);
}
