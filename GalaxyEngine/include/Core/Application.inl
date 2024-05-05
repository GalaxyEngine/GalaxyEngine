#pragma once
#include "Core/Application.h"
namespace GALAXY 
{
	inline void Core::Application::AddResourceToSend(const std::filesystem::path& fullPath)
	{
		if (fullPath.empty())
			return;
		if (std::ranges::find(m_resourceToSend, fullPath) == m_resourceToSend.end())
			m_resourceToSend.push_back(fullPath);
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

	bool* Core::Application::GetDrawGridPtr()
	{
		return &m_drawGrid;
	}
#endif
}
