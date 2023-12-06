#pragma once
#include <GalaxyAPI.h>
struct GLFWwindow;

#ifdef _WIN32
#include <Windows.h>
#endif

#include <galaxymath/Maths.h>
#include <functional>

namespace GALAXY
{
	namespace Wrapper {
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

		enum class CoordinateSpace
		{
			Window,
			Screen
		};

		class Window
		{
		public:
			~Window() = default;

			static bool Initialize();
			static void UnInitialize();

			static void PollEvent();
			static void* GetCurrentContext();
			static void MakeContextCurrent(void* window);
			static void* GetProcAddress(const char* procname);

			// --- Callbacks --- //
			static void ResizeCallback(GLFWwindow* window, int width, int height);
			static void MoveCallback(GLFWwindow* window, int xpos, int ypos);
			static void DropCallback(GLFWwindow* window, int count, const char** paths);

			void Create(const WindowConfig& config);
			void Destroy() const;

			void SwapBuffers() const;

			// --- Getters --- //
			bool GetVSyncEnable() const;
			void GetSize(int* width, int* height) const;
			Vec2i GetSize() const;
			Vec2i GetPosition() const;
			Vec2i GetMousePosition(CoordinateSpace coordinate = CoordinateSpace::Window) const;

			Vec2i ToWindowSpace(const Vec2i& pos) const;
			Vec2i ToScreenSpace(const Vec2i& pos) const;

			// --- Setters --- //
			void SetVSync(bool enable);
			void SetFullscreen(bool enable);
			void ToggleFullscreen();
			void SetSize(int width, int height) const;
			void SetSize(const Vec2i& size) const;
			void SetMousePosition(const Vec2i& pos, bool physicalPos = false) const;
			void SetCursorMode(CursorMode mode) const;

			bool ShouldClose() const;
			void Close() const;
			void ForceClose();
			void CancelClose();

			[[nodiscard]] void* GetWindow() const { return m_window; }

#ifdef _WIN32
			[[nodiscard]] HWND GetWindowWIN32() const;
#endif
			void ComputeScale();

			[[nodiscard]] float GetScreenScale() const;

			[[nodiscard]] void* GetCurrentMonitor() const;
			inline void SetShouldCloseCallback(std::function<void(bool)> val) { m_shouldCloseCallback = val; }
			inline void SetShouldDisplaySafeClose(std::function<bool()> val) { m_shouldDisplaySafeClose = val; }
		private:
			static int CursorModeToAPI(CursorMode mode);

			void* m_window = nullptr;
			bool m_vsync = true;

			float m_scale = 1.0f;

			std::function<void(bool)> m_shouldCloseCallback;
			std::function<bool()> m_shouldDisplaySafeClose = [] {return true; };
			bool m_forceClose;

		};
	}
}