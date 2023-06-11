#pragma once
#include <GalaxyAPI.h>

namespace GALAXY
{
	namespace Wrapper 
	{
		class Window;
	}
	namespace Core {
		class Application
		{
		public:
			~Application();

			static Application& GetInstance() { return m_instance; }

			void Initalize();
			void Update();
			void Destroy();

		private:
			static Application m_instance;
			std::unique_ptr<Wrapper::Window> m_window;
		};
	}
}