#pragma once
#include "Utils/FileWatcher.h"
#include <filesystem>

namespace GALAXY
{
	inline void Utils::FileWatcher::Update()
	{
		if (!m_multithread)
		{
			if (m_shouldStop)
				return;
			m_currentTime += Wrapper::GUI::DeltaTime();
			if (m_currentTime < 1.f)
				return;

			m_currentTime = 0.f;

			bool exist = std::filesystem::exists(m_filePath);
			if (m_exist && !exist) {
				// Existed but not now, Delete
				std::cerr << "File Deleted " << m_filePath << std::endl;
				StopWatching();
				return;
			}
			else if (!m_exist && exist)
			{
				// Not exist but now yes, callback to say that the file is created
				m_exist = exist;
				m_callback();
			}
			else if (!exist)
			{
				// If not exist
				return;
			}

			auto currentFileTime = std::filesystem::last_write_time(m_filePath);

			if (currentFileTime != m_lastModificationTime) {
				// Call callback only if the value was initalized
				if (m_lastModificationTime.has_value())
					m_callback();
				m_lastModificationTime = currentFileTime;
			}
		}
	}
}
