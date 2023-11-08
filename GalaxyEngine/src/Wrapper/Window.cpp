#include "pch.h"
#include "Wrapper/Window.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <GLFW/glfw3native.h>
#endif

namespace GALAXY {
	static bool s_initialized = false;
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
		const size_t major = 4, minor = 5;
		PrintLog("Initalized GLFW %d.%d", major, minor);
		glfwSetErrorCallback(glfw_error_callback);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		s_initialized = true;
		return true;
	}

	void Wrapper::Window::UnInitialize()
	{
		if (!s_initialized)
			return;
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

	void Wrapper::Window::SetMousePosition(const Vec2i& pos, bool physicalPos /*= false*/)
	{
    	GLFWwindow* window = static_cast<GLFWwindow*>(m_window);
		int cursorMode;
		cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (!physicalPos)
			glfwSetCursorPos(window, pos.x, pos.y);
		else
		{
			Vec2i windowPos;
			glfwGetWindowPos(window, &windowPos.x, &windowPos.y);
			glfwSetCursorPos(window, (double)(pos.x - windowPos.x), (double)(pos.y - windowPos.y));
		}
		// glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
	}

	int Wrapper::Window::CursorModeToAPI(CursorMode mode)
	{
		switch (mode)
		{
		case Wrapper::CursorMode::Normal:
			return GLFW_CURSOR_NORMAL;
		case Wrapper::CursorMode::Hidden:
			return GLFW_CURSOR_HIDDEN;
		case Wrapper::CursorMode::Disabled:
			return GLFW_CURSOR_DISABLED;
		default:
			PrintError("Cursor Mode not supported");
			return GLFW_CURSOR_NORMAL;
		}
	}

	void Wrapper::Window::SetCursorMode(CursorMode mode)
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		int modeValue = CursorModeToAPI(mode);
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, modeValue);
	}

	Math::Vec2i Wrapper::Window::GetPosition()
	{
		Vec2i windowPos;
		glfwGetWindowPos(static_cast<GLFWwindow*>(m_window), &windowPos.x, &windowPos.y);
		return windowPos;
	}

	void Wrapper::Window::Close()
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwSetWindowShouldClose(glfwWindow, true);
	}

#ifdef _WIN32
	HWND Wrapper::Window::GetWindowWIN32()
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		return glfwGetWin32Window(glfwWindow);
	}
#endif

}
