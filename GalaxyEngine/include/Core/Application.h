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
	namespace Editor { class ThumbnailCreator; }
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
			~Application() = default;

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
			Editor::ThumbnailCreator* GetThumbnailCreator() const { return m_thumbnailCreator; }
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
			Editor::ThumbnailCreator* m_thumbnailCreator = nullptr;
			Editor::Benchmark m_benchmark;


			Scripting::ScriptEngine* m_scriptEngine = nullptr;

			Unique<Wrapper::Window> m_window;

			std::deque<std::filesystem::path> m_resourceToSend;

			bool m_drawGrid = true;

			std::string m_clipboard;
		};
	}
}
#include "Core/Application.inl" 
