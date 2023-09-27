#pragma once
#include "GalaxyAPI.h"
#include <functional>
#include <chrono>
namespace GALAXY
{
	namespace Utils
	{
		class FileWatcher {
		public:
			FileWatcher(const std::string& filePath, std::function<void()> callback, bool multithread = false)
				: m_filePath(filePath), m_callback(callback), m_running(false), m_multithread(multithread) {
				m_lastModificationTime = std::filesystem::last_write_time(m_filePath);
			}
			~FileWatcher() { StopWatching(); }

			void StartWatching();

			inline void Update();

			void StopWatching() {
				m_shouldStop = false;
			}

		private:
			bool m_multithread = false;
			float m_currentTime = 0.f;


			std::string m_filePath;
			std::function<void()> m_callback;
			bool m_running;
			bool m_shouldStop = false;
			std::filesystem::file_time_type m_lastModificationTime;
		};
	}
}
#include "Utils/FileWatcher.inl" 
