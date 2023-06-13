#pragma once
#include <GalaxyAPI.h>
#include <memory>

namespace GALAXY
{
	namespace Resource	{ class ResourceManager; }
	namespace Wrapper { class Window; class Renderer; }
	namespace Core 
	{
		class GALAXY_API Application
		{
		public:
			~Application();

			static Application& GetInstance() { return m_instance; }

			void Initalize();
			void Update();
			void Destroy();

		private:
			static Application m_instance;
			Resource::ResourceManager* m_resourceManager = nullptr;
			Wrapper::Renderer* m_renderer = nullptr;
			std::unique_ptr<Wrapper::Window> m_window;
		};
	}
}