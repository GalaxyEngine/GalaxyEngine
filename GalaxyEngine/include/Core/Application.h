#pragma once
#include <GalaxyAPI.h>

#include <Wrapper/Window.h>
#include <Utils/Type.h>

#include "Editor/EditorSettings.h"

#include <deque>
#include <filesystem>

#include "Editor/Benchmark.h"

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

			static inline Application& GetInstance();

			void Initialize(const std::filesystem::path& projectPath);
			void Update();

			void PasteObject() const;

			void CopyObject();

			void Destroy() const;

			inline void AddResourceToSend(const std::filesystem::path& fullPath);

			void UpdateResources();

			void TrySendResource(Shared<Resource::IResource> resource, const std::filesystem::path& resourcePath);

			[[nodiscard]] inline Wrapper::Window* GetWindow() const;

			inline Editor::EditorSettings& GetEditorSettings();
			inline Editor::Benchmark& GetBenchmark();

			inline bool* GetDrawGridPtr();

			void Exit() const;
		private:
			static Application m_instance;

			Resource::ResourceManager* m_resourceManager = nullptr;
			Wrapper::Renderer* m_renderer = nullptr;

			Core::ThreadManager* m_threadManager = nullptr;
			Core::SceneHolder* m_sceneHolder = nullptr;

			Editor::UI::EditorUIManager* m_editorUI = nullptr;
			Editor::EditorSettings m_editorSettings;
			Scripting::ScriptEngine* m_scriptEngine = nullptr;

			Render::LightManager* m_lightManager = nullptr;

			Unique<Wrapper::Window> m_window;

			std::deque<std::filesystem::path> m_resourceToSend;

			bool m_drawGrid = true;
			
			std::string m_clipboard;

			Editor::Benchmark m_benchmark;
		};
	}
}
#include "Core/Application.inl" 
