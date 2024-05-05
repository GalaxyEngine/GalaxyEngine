#pragma once
#include <GalaxyAPI.h>

#ifdef WITH_EDITOR
#include "Editor/UI/EditorUIManager.h"
#endif

struct GLFWwindow;

#ifdef _WIN32
#include <Windows.h>
#endif

#include <filesystem>

#include "Utils/Type.h"
#include "Utils/Event.h"

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

		struct VideoMode
		{
			VideoMode() = default;
			VideoMode(int width, int height, int refreshRate) : size(width, height), refreshRate(refreshRate) {}
			VideoMode(const Vec2i& size, int refreshRate) : size(size), refreshRate(refreshRate) {}

			Vec2i size;
			int refreshRate;
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
			static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
			static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
			static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
			static void ScaleCallback(GLFWwindow* window, float xScale, float yScale);

			void Create(const WindowConfig& config);
			void Destroy() const;

			void SwapBuffers() const;

			// --- Getters --- //
			bool IsVSyncEnable() const;
			void GetSize(int* width, int* height) const;
			Vec2i GetSize() const;
			Vec2i GetPosition() const;
			Vec2i GetMousePosition(CoordinateSpace coordinate = CoordinateSpace::Window) const;
			List<VideoMode> GetSupportedFullScreenResolutions() const;
			VideoMode GetVideoMode() const;

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

			void SetIcon(const std::filesystem::path& pathToIcon);

			bool ShouldClose();
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
		public:
			Utils::Event<const Vec2i& /*pos*/> EOnMove;
			Utils::Event<const Vec2f& /*dpi*/> EOnDPIChange;
			Utils::Event<const Vec2i& /*size*/> EOnResize;
		private:
			static int CursorModeToAPI(CursorMode mode);

			void* m_window = nullptr;
			bool m_vsync = true;

			float m_scale = 1.0f;
			
			bool m_forceClose = false;

		};
	}
}