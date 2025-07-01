#pragma once
#include "GalaxyAPI.h"
namespace GALAXY
{
	namespace Utils
	{
		class GALAXY_API Time
		{
		public:
			Time(const uint64_t time = 0) : m_time(time) {}
			Time(const std::chrono::steady_clock::time_point& tp);

			static Time FromSeconds(float seconds);
			static Time FromMilliseconds(uint64_t milliseconds);
			static Time FromMinutes(float minutes);
			static Time FromHours(float hours);

			uint64_t AsMilliseconds() const;
			double AsSeconds() const;
			double AsMinutes() const;
			double AsHours() const;

			operator uint64_t() const { return m_time; }

			static float DeltaTime();
			static double TimeSinceStart();

			static void UpdateDeltaTime();

			static std::string FormatTimeSeconds(float seconds, const std::string& format = "hh:mm:ss");

		private:
			static float s_deltaTime;
			static double s_lastTime;

			uint64_t m_time = 0;
		};
		
		class GALAXY_API ElapsedTimer
		{
		public:
			ElapsedTimer();

			void Start();
			void Stop();

			Time GetElapsedTime() const;
		private:
			Time m_startPoint;
			Time m_elapsedTime;
			bool m_started = false;
		};
	}
}