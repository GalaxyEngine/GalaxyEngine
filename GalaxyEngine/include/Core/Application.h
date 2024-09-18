#pragma once
#include <GalaxyAPI.h>

#include <Wrapper/Window.h>
#include <Utils/Type.h>

#ifdef WITH_EDITOR
#include "Editor/EditorSettings.h"
#include "Editor/Benchmark.h"
#endif

#include "Core/ProjectSettings.h"

#include <deque>
#include <filesystem>


namespace GALAXY
{
	namespace Editor { class ThumbnailCreator; }
	namespace Resource { class ResourceManager; class IResource; }
	namespace Wrapper {
		class PhysicsWrapper;
		class Window; class Renderer; class Audio; }
#ifdef WITH_EDITOR
	namespace Editor::UI { class EditorUIManager; }
#endif
	namespace Scripting { class ScriptEngine; }
	namespace Render { class LightManager; }
	namespace Core
	{
		class ThreadManager;
		class SceneHolder;
		class GALAXY_API Application
		{
		public:
			~Application() = default;

			static inline Application& GetInstance();

			void Initialize(std::filesystem::path projectPath);
			void Update();

			void Destroy() const;

			inline void AddResourceToSend(const std::filesystem::path& fullPath);

			void UpdateResources();

			void TrySendResource(const Shared<Resource::IResource>& resource, const std::filesystem::path& resourcePath);

			[[nodiscard]] inline Wrapper::Window* GetWindow() const;

#ifdef WITH_EDITOR
			void PasteObject() const;
			void CopyObject();

			inline Editor::EditorSettings& GetEditorSettings();
			inline Editor::Benchmark& GetBenchmark();
			Editor::ThumbnailCreator* GetThumbnailCreator() const { return m_thumbnailCreator; }
			inline bool* GetDrawGridPtr();

			static bool IsEditorMode() { return m_instance.m_applicationMode == Editor::ApplicationMode::Editor; }
			static bool IsPlayMode() { return m_instance.m_applicationMode == Editor::ApplicationMode::Play; }
			static bool IsPauseMode() { return m_instance.m_applicationMode == Editor::ApplicationMode::Pause; }

			void SetApplicationMode(Editor::ApplicationMode mode);
#endif
			inline ProjectSettings& GetProjectSettings();

			void Exit() const;
		private:
			static Application m_instance;

			Resource::ResourceManager* m_resourceManager = nullptr;
			Wrapper::Renderer* m_renderer = nullptr;
			Wrapper::Audio* m_audioSystem = nullptr;
			Wrapper::PhysicsWrapper* m_physicsWrapper = nullptr;

			Core::ThreadManager* m_threadManager = nullptr;
			Core::SceneHolder* m_sceneHolder = nullptr;
			Core::ProjectSettings m_projectSettings;

#ifdef WITH_EDITOR
			Editor::UI::EditorUIManager* m_editorUI = nullptr;
			Editor::EditorSettings m_editorSettings;
			Editor::ThumbnailCreator* m_thumbnailCreator = nullptr;
			Editor::Benchmark m_benchmark;
			bool m_drawGrid = true;

			Editor::ApplicationMode m_applicationMode = Editor::ApplicationMode::Editor;
#endif

			Scripting::ScriptEngine* m_scriptEngine = nullptr;

			Unique<Wrapper::Window> m_window;

			std::deque<std::filesystem::path> m_resourceToSend;

			std::string m_clipboard;
		};
	}
}
#include "Core/Application.inl" 
