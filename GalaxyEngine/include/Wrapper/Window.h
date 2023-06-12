#pragma once
#include <GalaxyAPI.h>
#include <Maths/Maths.h>

namespace GALAXY::Wrapper
{
	struct WindowConfig
	{
		const char* name;
		int width, height;
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

		void Create(const WindowConfig& config);
		void Destroy();

		void SwapBuffers();

		// --- Getters --- //
		void GetSize(int* width, int* height);
		Vec2i GetSize();

		// --- Setters --- //
		void SetVSync(bool enable);
		void SetSize(int width, int height);
		void SetSize(const Vec2i& size);

		bool ShouldClose();

		[[nodiscard]] void* GetWindow() { return m_window; }

	private:
		void* m_window;

	};
}