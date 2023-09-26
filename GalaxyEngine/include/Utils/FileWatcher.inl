#pragma once
#include "Utils/FileWatcher.h"
#include <filesystem>

namespace GALAXY
{
	inline void Utils::FileWatcher::Update()
	{
		if (m_running) {
			std::cout << "FileWatcher is already running." << std::endl;
			return;
		}

		m_running = true;

		auto lastModificationTime = std::filesystem::last_write_time(m_filePath);

		bool exist = std::filesystem::exists(m_filePath);
		if (!exist) {
			std::cerr << "File Deleted " << m_filePath << std::endl;
			StopWatching();
			return;
		}

		auto currentFileTime = std::filesystem::last_write_time(m_filePath);

		if (currentFileTime != lastModificationTime) {
			lastModificationTime = currentFileTime;
			m_callback();
		}

		// Check for changes every 1 second
		std::this_thread::sleep_for(std::chrono::seconds(1));

		StartWatching();
	}
}
