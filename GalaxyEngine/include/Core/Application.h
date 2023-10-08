#pragma once
#include <GalaxyAPI.h>

#include <Wrapper/Window.h>

#include <memory>
#include <deque>
#include <filesystem>

namespace GALAXY
{
	namespace Resource	{ class ResourceManager; }
	namespace Wrapper { class Window; class Renderer; }
	namespace EditorUI { class EditorUIManager; }
	namespace Scripting { class ScriptEngine; }
	namespace Core 
	{
		class GALAXY_API Application
		{
		public:
			~Application();

			static Application& GetInstance() { return m_instance; }

			void Initalize(const std::filesystem::path& projectPath);
			void Update();
			void Destroy();

			void AddResourceToSend(const std::filesystem::path& fullPath);

			void UpdateResources();

			Wrapper::Window* GetWindow() { return m_window.get(); }

			bool* GetDrawGrid() { return &m_drawGrid; }

			void Exit();

		private:
			static Application m_instance;

			Resource::ResourceManager* m_resourceManager = nullptr;
			Wrapper::Renderer* m_renderer = nullptr;
			class ThreadManager* m_threadManager = nullptr;
			class SceneHolder* m_sceneHolder = nullptr;
			EditorUI::EditorUIManager* m_editorUI = nullptr;
			Scripting::ScriptEngine* m_scriptEngine = nullptr;

			std::unique_ptr<Wrapper::Window> m_window;

			std::deque<std::filesystem::path> m_resourceToSend;

			bool m_drawGrid = true;
		};
	}
}
