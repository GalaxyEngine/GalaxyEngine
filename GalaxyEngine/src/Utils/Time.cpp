#include "pch.h"
#include "Utils/Time.h"
namespace GALAXY::Utils
{
	float Time::s_deltaTime = 0.0f;

	Time Time::FromSeconds(const float seconds)
	{
		return { static_cast<uint64_t>(seconds * 1'000) };
	}

	Time Time::FromMilliseconds(const float milliseconds)
	{
		return { static_cast<uint64_t>(milliseconds) };
	}

	Time Time::FromMinutes(const float minutes)
	{
		return { static_cast<uint64_t>(minutes * 60'000) };
	}

	Time Time::FromHours(const float hours)
	{
		return { static_cast<uint64_t>(hours * 3'600'000) };
	}

	uint64_t Time::AsMilliseconds() const
	{
		return m_time;
	}

	double Time::AsSeconds() const
	{
		return static_cast<double>(m_time) / 1'000.0;
	}

	double Time::AsMinutes() const
	{
		return static_cast<double>(m_time) / 60'000.0;
	}

	double Time::AsHours() const
	{
		return static_cast<double>(m_time) / 60'000'000.0;
	}

	float Time::DeltaTime()
	{
		return s_deltaTime;
	}

	void Time::UpdateDeltaTime()
	{
		s_deltaTime = Wrapper::GUI::DeltaTime();
	}
}
