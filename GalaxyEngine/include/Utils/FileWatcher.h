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
				: m_filePath(filePath), m_callback(callback), m_multithread(multithread), m_exist(std::filesystem::exists(filePath)){
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
			bool m_shouldStop = false;
			bool m_exist = false;
			std::optional<std::filesystem::file_time_type> m_lastModificationTime;
		};
	}
}
#include "Utils/FileWatcher.inl" 
