#pragma once
#include <GalaxyAPI.h>

#include <Wrapper/Window.h>
#include <Utils/Type.h>

#include <deque>
#include <filesystem>

namespace GALAXY
{
	namespace Resource { class ResourceManager; class IResource; }
	namespace Wrapper { class Window; class Renderer; }
	namespace Editor::UI { class EditorUIManager; }
	namespace Scripting { class ScriptEngine; }
	namespace Render { class LightManager; }
	namespace Core 
	{
		class ThreadManager;
		class SceneHolder;
		class GALAXY_API Application
		{
		public:
			~Application();

			static Application& GetInstance() { return m_instance; }

			void Initialize(const std::filesystem::path& projectPath);
			void Update();

			void PasteObject();

			void CopyObject();

			void Destroy();

			inline void AddResourceToSend(const std::filesystem::path& fullPath);

			void UpdateResources();

			void TrySendResource(Shared<Resource::IResource> resource, const std::filesystem::path& resourcePath);

			inline Wrapper::Window* GetWindow() { return m_window.get(); }

			inline bool* GetDrawGridPtr() { return &m_drawGrid; }

			void Exit();

		private:
			static Application m_instance;

			Resource::ResourceManager* m_resourceManager = nullptr;
			Wrapper::Renderer* m_renderer = nullptr;

			Core::ThreadManager* m_threadManager = nullptr;
			Core::SceneHolder* m_sceneHolder = nullptr;

			Editor::UI::EditorUIManager* m_editorUI = nullptr;
			Scripting::ScriptEngine* m_scriptEngine = nullptr;

			Render::LightManager* m_lightManager = nullptr;

			Unique<Wrapper::Window> m_window;

			std::deque<std::filesystem::path> m_resourceToSend;

			bool m_drawGrid = true;
			
			std::string m_clipboard;
		};
	}
}
#include "Core/Application.inl" 
