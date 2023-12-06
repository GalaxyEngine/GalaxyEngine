#include "pch.h"
#include "Wrapper/Window.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <GLFW/glfw3native.h>
#endif

#include "Core/Application.h"
#include "Editor/UI/EditorUIManager.h"

namespace GALAXY {
	// boolean to check if glfw has been initalized
	static bool s_initialized = false;

	static void glfw_error_callback(const int error, const char* description)
	{
		PrintError("GLFW Error %d: %s", error, description);
	}

	bool Wrapper::Window::Initialize()
	{
		if (!glfwInit()) {
			PrintError("Failed to initalize GLFW");
			return false;
		}
		constexpr size_t major = 4, minor = 5;
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
		glfwSetWindowPosCallback(glfwWindow, &MoveCallback);
		glfwSetDropCallback(glfwWindow, &DropCallback);

		ComputeScale();
	}

	void Wrapper::Window::Destroy() const
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwDestroyWindow(glfwWindow);
	}

	void Wrapper::Window::SwapBuffers() const
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwSwapBuffers(glfwWindow);
	}

	bool Wrapper::Window::GetVSyncEnable() const
	{
		return m_vsync;
	}

	void Wrapper::Window::GetSize(int* width, int* height) const
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwGetWindowSize(glfwWindow, width, height);
	}

	Vec2i Wrapper::Window::GetSize() const
	{
		Vec2i size;
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwGetWindowSize(glfwWindow, &size.x, &size.y);
		return size;
	}

	void Wrapper::Window::SetVSync(const bool enable)
	{
		m_vsync = enable;
		glfwSwapInterval(enable); // Enable vsync
	}

	bool Wrapper::Window::ShouldClose() const
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		bool shouldClose = glfwWindowShouldClose(glfwWindow);
		if (shouldClose && m_shouldDisplaySafeClose()) {
			m_shouldCloseCallback(shouldClose);
			return shouldClose && m_forceClose;
		}
		else
		{
			return shouldClose;
		}
	}

	void Wrapper::Window::SetSize(int width, int height) const
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwSetWindowSize(glfwWindow, width, height);
	}

	void Wrapper::Window::SetSize(const Vec2i& size) const
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

	void Wrapper::Window::SetFullscreen(const bool enable)
	{
		static Vec2i prevSize;
		static Vec2i prevPos;
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		if (enable) {
			prevPos = GetPosition();
			prevSize = GetSize();
			GLFWmonitor* primary = static_cast<GLFWmonitor*>(GetCurrentMonitor());

			const GLFWvidmode* mode = glfwGetVideoMode(primary);

			glfwSetWindowMonitor(glfwWindow, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
			SetVSync(m_vsync);
		}
		else
		{
			GLFWmonitor* primary = static_cast<GLFWmonitor*>(GetCurrentMonitor());
			const GLFWvidmode* mode = glfwGetVideoMode(primary);
			glfwSetWindowMonitor(glfwWindow, nullptr, 0, 0, prevSize.x, prevSize.y, 0);
			glfwSetWindowPos(glfwWindow, prevPos.x, prevPos.y);
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

	void Wrapper::Window::MoveCallback(GLFWwindow* window, int /*xpos*/, int /*ypos*/)
	{
		Core::Application::GetInstance().GetWindow()->ComputeScale();
	}

	void Wrapper::Window::DropCallback(GLFWwindow* window, const int count, const char** paths)
	{
		Editor::UI::EditorUIManager::GetInstance()->GetFileExplorer()->HandleDropFile(count, paths);
	}

	void Wrapper::Window::SetMousePosition(const Vec2i& pos, const bool physicalPos /*= false*/) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(m_window);
		int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (!physicalPos)
			glfwSetCursorPos(window, pos.x, pos.y);
		else
		{
			Vec2i windowPos;
			glfwGetWindowPos(window, &windowPos.x, &windowPos.y);
			glfwSetCursorPos(window, (double)(pos.x - windowPos.x), (double)(pos.y - windowPos.y));
		}
	}

	int Wrapper::Window::CursorModeToAPI(const CursorMode mode)
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

	void Wrapper::Window::SetCursorMode(const CursorMode mode) const
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		const int modeValue = CursorModeToAPI(mode);
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, modeValue);
	}

	Math::Vec2i Wrapper::Window::GetPosition() const
	{
		Vec2i windowPos;
		glfwGetWindowPos(static_cast<GLFWwindow*>(m_window), &windowPos.x, &windowPos.y);
		return windowPos;
	}

	Vec2i Wrapper::Window::GetMousePosition(CoordinateSpace coordinate) const
	{
		Vec2d mousePos;
		glfwGetCursorPos(static_cast<GLFWwindow*>(m_window), &mousePos.x, &mousePos.y);

		if (coordinate == CoordinateSpace::Screen)
		{
			mousePos = ToScreenSpace(mousePos);
		}
		return mousePos;
	}

	Vec2i Wrapper::Window::ToWindowSpace(const Vec2i& pos) const
	{
		const Vec2i windowPos = GetPosition();
		return pos - windowPos;
	}

	Vec2i Wrapper::Window::ToScreenSpace(const Vec2i& pos) const
	{
		const Vec2i windowPos = GetPosition();
		return pos + windowPos;
	}

	void Wrapper::Window::Close() const
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwSetWindowShouldClose(glfwWindow, true);
	}

	void Wrapper::Window::ForceClose()
	{
		m_forceClose = true;
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwSetWindowShouldClose(glfwWindow, true);
	}

	void Wrapper::Window::CancelClose()
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		glfwSetWindowShouldClose(glfwWindow, false);
	}

#ifdef _WIN32
	HWND Wrapper::Window::GetWindowWIN32() const
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		return glfwGetWin32Window(glfwWindow);
	}
#endif

	void Wrapper::Window::ComputeScale()
	{
		float yScale = 0;
		glfwGetMonitorContentScale(static_cast<GLFWmonitor*>(GetCurrentMonitor()), &m_scale, &yScale);
	}

	float Wrapper::Window::GetScreenScale() const
	{
		return m_scale;
	}

	void* Wrapper::Window::GetCurrentMonitor() const
	{
		int nmonitors;
		int wx, wy, ww, wh;
		int mx, my;
		int bestoverlap;
		GLFWmonitor* bestmonitor;

		bestoverlap = 0;
		bestmonitor = NULL;

		glfwGetWindowPos(static_cast<GLFWwindow*>(m_window), &wx, &wy);
		glfwGetWindowSize(static_cast<GLFWwindow*>(m_window), &ww, &wh);
		GLFWmonitor** monitors = glfwGetMonitors(&nmonitors);

		for (int i = 0; i < nmonitors; i++) {
			const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
			glfwGetMonitorPos(monitors[i], &mx, &my);
			const int mw = mode->width;
			const int mh = mode->height;

			const int overlap = std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) *
				std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

			if (bestoverlap < overlap) {
				bestoverlap = overlap;
				bestmonitor = monitors[i];
			}
		}

		return bestmonitor;
	}

}
