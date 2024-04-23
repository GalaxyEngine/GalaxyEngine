#pragma once
#include "GalaxyAPI.h"
namespace GALAXY
{
	namespace Utils {

		class Time
		{
		public:
			Time(const uint64_t time = 0) : m_time(time) {}

			static Time FromSeconds(float seconds);
			static Time FromMilliseconds(float milliseconds);
			static Time FromMinutes(float minutes);
			static Time FromHours(float hours);

			uint64_t AsMilliseconds() const;
			double AsSeconds() const;
			double AsMinutes() const;
			double AsHours() const;

			operator uint64_t() const { return m_time; }

			static float DeltaTime();

			static void UpdateDeltaTime();

			static std::string FormatTimeSeconds(float seconds, const std::string& format = "hh:mm:ss");

		private:
			static float s_deltaTime;

			uint64_t m_time = 0;
		};
	}
}