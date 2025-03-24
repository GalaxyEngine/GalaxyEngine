#pragma once
#include "Core/Application.h"
namespace GALAXY 
{
	inline void Core::Application::AddResourceToSend(const std::filesystem::path& fullPath)
	{
		if (fullPath.empty())
			return;
		Core::ThreadManager::Lock();
		for (const std::filesystem::path& path : m_resourceToSend)
		{
			if (path == fullPath)
			{
				Core::ThreadManager::Unlock();
				return;
			}
		}
		m_resourceToSend.push_back(fullPath);
		Core::ThreadManager::Unlock();
	}

	Wrapper::Window* Core::Application::GetWindow() const
	{
		return m_window.get();
	}

	Core::Application& Core::Application::GetInstance()
	{
		return m_instance;
	}

	Core::ProjectSettings& Core::Application::GetProjectSettings()
	{
		return m_projectSettings;
	}

#ifdef WITH_EDITOR
	Editor::EditorSettings& Core::Application::GetEditorSettings()
	{
		return m_editorSettings;
	}

	Editor::Benchmark& Core::Application::GetBenchmark()
	{
		return m_benchmark;
	}
#endif
}
