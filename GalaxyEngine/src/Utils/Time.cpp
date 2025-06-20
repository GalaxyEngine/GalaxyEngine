#include "pch.h"
#include "Utils/Time.h"
namespace GALAXY::Utils
{
	float Time::s_deltaTime = 0.0f;

	Time::Time(const std::chrono::steady_clock::time_point& tp)
	{
		int64_t milliseconds = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count());
		m_time = FromMilliseconds(milliseconds);
	}

	Time Time::FromSeconds(const float seconds)
	{
		return { static_cast<uint64_t>(seconds * 1'000) };
	}

	Time Time::FromMilliseconds(const uint64_t milliseconds)
	{
		return { milliseconds };
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
		s_deltaTime = Wrapper::UI::DeltaTime();
	}

	std::string Time::FormatTimeSeconds(float seconds, const std::string& format)
	{
		std::string out;
		out = format;
		int hours = static_cast<int>(seconds) / 3600;
		int minutes = (static_cast<int>(seconds) / 60) % 60;
		int secs = static_cast<int>(seconds) % 60;
		auto it = format.find("hh");
		if (it != std::string::npos)
		{
			auto str = std::to_string(hours);
			if (hours < 10)
				str = "0" + str;
			if (hours == 0)
				str = "00";
			out.replace(it, 2, str);
		}
		it = format.find("mm");
		if (it != std::string::npos)
		{
			auto str = std::to_string(minutes);
			if (minutes < 10)
				str = "0" + str;
			if (minutes == 0)
				str = "00";
			out.replace(it, 2, str);
		}
		it = format.find("ss");
		if (it != std::string::npos)
		{
			auto str = std::to_string(secs);
			if (secs < 10)
				str = "0" + str;
			if (secs == 0)
				str = "00";
			out.replace(it, 2, str);
		}
		return out;
	}

	ElapsedTimer::ElapsedTimer()
	{
		Start();
	}

	void ElapsedTimer::Start()
	{
		m_started = true;
		m_startPoint = Time(std::chrono::steady_clock::now());
	}

	void ElapsedTimer::Stop()
	{
		m_started = false;
		m_elapsedTime = GetElapsedTime();
	}

	Time ElapsedTimer::GetElapsedTime() const
	{
		Time currentTime(std::chrono::steady_clock::now());
		return currentTime - m_startPoint;
	}
}
