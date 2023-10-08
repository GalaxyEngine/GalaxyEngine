#pragma once
#include <GalaxyAPI.h>
#include <Maths/Maths.h>

struct GLFWwindow;
namespace GALAXY::Wrapper
{
	struct WindowConfig
	{
		const char* name;
		int width, height;
	};
	enum class CursorType
	{
		Arrow,
		IBeam,
		CrossHair,
		Hand,
		HResize,
		WResize
	};

	enum class CursorMode
	{
		Normal,
		Hidden,
		Disabled
	};

	class Window
	{
	public:
		~Window();

		static bool Initialize();
		static void UnInitialize();

		static void PollEvent();
		static void* GetCurrentContext();
		static void MakeContextCurrent(void* window);
		static void* GetProcAddress(const char* procname);

		// --- Callbacks --- //
		static void ResizeCallback(GLFWwindow* window, int width, int height);

		void Create(const WindowConfig& config);
		void Destroy();

		void SwapBuffers();

		// --- Getters --- //
		bool GetVSyncEnable();
		void GetSize(int* width, int* height);
		Vec2i GetSize();
		Vec2i GetPosition();

		// --- Setters --- //
		void SetVSync(bool enable);
		void SetFullscreen(bool enable);
		void ToggleFullscreen();
		void SetSize(int width, int height);
		void SetSize(const Vec2i& size);
		void SetMousePosition(const Vec2i& pos, bool physicalPos = false);
		void SetCursorMode(CursorMode mode);

		bool ShouldClose();
		void Close();

		[[nodiscard]] void* GetWindow() { return m_window; }

	private:
		int CursorModeToAPI(CursorMode mode);

		void* m_window;
		bool m_vsync = true;

	};
}