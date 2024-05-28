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

#ifdef WITH_EDITOR
#include "Editor/UI/EditorUIManager.h"
#endif

#include "Wrapper/ImageLoader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

#include "Core/Input.h"

namespace GALAXY {
	// boolean to check if glfw has been initialized
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
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); 
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
		glfwSetKeyCallback(glfwWindow, &KeyCallback);
		glfwSetMouseButtonCallback(glfwWindow, &MouseButtonCallback);
		glfwSetScrollCallback(glfwWindow, &ScrollCallback);
		glfwSetWindowContentScaleCallback(glfwWindow, &ScaleCallback);

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

	bool Wrapper::Window::IsVSyncEnable() const
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

	bool Wrapper::Window::ShouldClose()
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_window);
		bool shouldClose = glfwWindowShouldClose(glfwWindow);
		if (shouldClose && Editor::UI::EditorUIManager::ShouldDisplaySafeClose()) {
			auto eventOnValidate = [this] { ForceClose(); };
			Editor::UI::EditorUIManager::GetInstance()->SetOnValidatePopupEvent(eventOnValidate);
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

	void Wrapper::Window::MoveCallback(GLFWwindow* window, int xpos, int ypos)
	{
		auto self = Core::Application::GetInstance().GetWindow();
		self->EOnMove.Invoke(Vec2i(xpos, ypos));
	}

	void Wrapper::Window::DropCallback(GLFWwindow* window, const int count, const char** paths)
	{
#ifdef WITH_EDITOR
		Editor::UI::EditorUIManager::GetInstance()->GetFileExplorer()->HandleDropFile(count, paths);
#endif
	}

	void Wrapper::Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		const KeyEvent state = action == GLFW_PRESS ? KeyEvent::Pressed : action == GLFW_RELEASE ? KeyEvent::Released : KeyEvent::None;
		Input::key_callback(state, key);
	}

	void Wrapper::Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		const KeyEvent state = action == GLFW_PRESS ? KeyEvent::Pressed : action == GLFW_RELEASE ? KeyEvent::Released : KeyEvent::None;
		Input::key_callback(state, button);
	}

	void Wrapper::Window::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		Input::scroll_callback(yoffset);
	}

	void Wrapper::Window::ScaleCallback(GLFWwindow* window, float xScale, float yScale)
	{
		auto self = Core::Application::GetInstance().GetWindow();
		self->ComputeScale();
		self->EOnDPIChange.Invoke(Vec2f(xScale, yScale));
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

	void Wrapper::Window::SetIcon(const std::filesystem::path& pathToIcon)
	{
		int number = 1;
		Wrapper::Image image = Wrapper::ImageLoader::Load(pathToIcon.string().c_str(), 4);

		if (image.data) {
			GLFWimage icons[1];

			icons[0].width = image.size.x;
			icons[0].height = image.size.y;
			icons[0].pixels = image.data;

			glfwSetWindowIcon(static_cast<GLFWwindow*>(m_window), 1, icons);

				Wrapper::ImageLoader::ImageFree(image);
		}
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

	List<Wrapper::VideoMode> Wrapper::Window::GetSupportedFullScreenResolutions() const
	{
		List<VideoMode> resolutions;
		int count;
		const GLFWvidmode* modes = glfwGetVideoModes(static_cast<GLFWmonitor*>(GetCurrentMonitor()), &count);
		resolutions.resize(count);
		for (int i = 0; i < count; i++)
		{
			resolutions[i] = VideoMode(modes[i].width, modes[i].height, modes[i].refreshRate);
		}
		return resolutions;
	}

	Wrapper::VideoMode Wrapper::Window::GetVideoMode() const
	{
		const GLFWvidmode* mode = glfwGetVideoMode(static_cast<GLFWmonitor*>(GetCurrentMonitor()));
		return { mode->width, mode->height, mode->refreshRate };
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
		// Check window size for null
		auto windowSize = GetSize();
		if (windowSize.x <= 0 || windowSize.y <= 0)
			return;
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
