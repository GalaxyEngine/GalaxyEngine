#include "GalaxyAPI.h"

#include <iostream>
#include <string>
#include <cstdio>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#endif

#define MAX_LOG_SIZE 1024

namespace GALAXY::Debug {

	enum class LogType
	{
		L_INFO,
		L_WARNING,
		L_ERROR
	};

	class GALAXY_API Log
	{
	public:
		~Log();
		template <typename ...Args> static void Print(const char* file, int line, LogType type, const char* format, Args ... args)
		{
			const std::time_t now = std::time(nullptr); // get the current time point
			const std::tm calendar_time = *std::localtime(std::addressof(now));

			char buf[MAX_LOG_SIZE];
			char buf2[MAX_LOG_SIZE];
#ifdef _WIN32
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			sprintf_s(buf2, format, args ...);
			sprintf_s(buf, "[%02d:%02d:%02d] %s (l:%d): %s", calendar_time.tm_hour, calendar_time.tm_min, calendar_time.tm_sec, file, line, buf2);
			switch (type)
			{
			case Debug::LogType::L_INFO:
				SetConsoleTextAttribute(hConsole, 15);
				break;
			case Debug::LogType::L_WARNING:
				SetConsoleTextAttribute(hConsole, 14);
				break;
			case Debug::LogType::L_ERROR:
				SetConsoleTextAttribute(hConsole, 4);
				break;
			default:
				break;
			}
			std::cout << buf << '\n';
			SetConsoleTextAttribute(hConsole, 15);
#else
			snprintf(buf2, sizeof(buf2), format, args...);
			snprintf(buf, sizeof(buf), "[%02d:%02d:%02d] %s (l:%d): %s",
				calendar_time.tm_hour, calendar_time.tm_min, calendar_time.tm_sec,
				file, line, buf2);
			switch (type)
			{
			case LogType::L_INFO:
				std::cout << "\033[37m"; // Set console text color to white
				break;
			case LogType::L_WARNING:
				std::cout << "\033[33m"; // Set console text color to yellow
				break;
			case LogType::L_ERROR:
				std::cout << "\033[31m"; // Set console text color to red
				break;
			default:
				break;
			}

			std::cout << buf << std::endl;
			std::cout << "\033[0m"; // Reset console text color
#endif
			//AddTextToConsole(type, buf);
		}

		static void AddTextToConsole(LogType type, const std::string& text);
	};
}

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define LOG(t, x, ...) Debug::Log::Print(__FILENAME__, __LINE__, t, x, ##__VA_ARGS__);
#define PrintLog(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_INFO, x, ##__VA_ARGS__);
#define PrintWarning(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_WARNING, x, ##__VA_ARGS__);
#define PrintError(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_ERROR, x, ##__VA_ARGS__);

#define ASSERT(condition) \
        assert(condition); \