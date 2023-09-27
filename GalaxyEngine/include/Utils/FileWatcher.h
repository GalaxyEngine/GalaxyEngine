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
			FileWatcher(const std::string& filePath, std::function<void()> callback)
				: m_filePath(filePath), m_callback(callback), m_running(false) {}

			void StartWatching();

			inline void Update();

			void StopWatching() {
				m_running = false;
			}

		private:
			std::string m_filePath;
			std::function<void()> m_callback;
			bool m_running;
			std::filesystem::file_time_type m_lastModificationTime;
		};
	}
}
#include "Utils/FileWatcher.inl" 
