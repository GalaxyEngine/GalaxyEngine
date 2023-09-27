#pragma once
#include "Utils/FileWatcher.h"
#include <filesystem>

namespace GALAXY
{
	inline void Utils::FileWatcher::Update()
	{
		if (m_multithread)
		{
			if (m_shouldStop)
				return;

			if (!m_running) {
				m_running = true;
				m_lastModificationTime = std::filesystem::last_write_time(m_filePath);
			}

			bool exist = std::filesystem::exists(m_filePath);
			if (!exist) {
				std::cerr << "File Deleted " << m_filePath << std::endl;
				StopWatching();
				return;
			}

			auto currentFileTime = std::filesystem::last_write_time(m_filePath);

			if (currentFileTime != m_lastModificationTime) {
				m_lastModificationTime = currentFileTime;
				m_callback();
			}

			// Check for changes every 1 second
			std::this_thread::sleep_for(std::chrono::seconds(1));

			StartWatching();
		}
		else
		{
			if (m_shouldStop)
				return;
			m_currentTime += Wrapper::GUI::DeltaTime();
			if (m_currentTime < 1.f)
				return;

			m_currentTime = 0.f;

			bool exist = std::filesystem::exists(m_filePath);
			if (!exist) {
				std::cerr << "File Deleted " << m_filePath << std::endl;
				StopWatching();
				return;
			}

			auto currentFileTime = std::filesystem::last_write_time(m_filePath);

			if (currentFileTime != m_lastModificationTime) {
				m_lastModificationTime = currentFileTime;
				m_callback();
			}
		}
	}
}
