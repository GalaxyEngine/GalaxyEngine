#include "GalaxyAPI.h"

#include <iostream>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define MAX_LOG_SIZE 1024

namespace GALAXY::Debug {

	enum class LogType
	{
		L_INFO,
		L_WARNING,
		L_ERROR
	};

	class Log
	{
	public:
		~Log();
		template <typename ...Args> static void Print(const char* file, int line, LogType type, const char* format, Args ... args)
		{
			const std::time_t now = std::time(nullptr); // get the current time point
			const std::tm calendar_time = *std::localtime(std::addressof(now));

			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			char buf[MAX_LOG_SIZE];
			char buf2[MAX_LOG_SIZE];
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
		}
	};
}

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define LOG(t, x, ...) Debug::Log::Print(__FILENAME__, __LINE__, t, x, __VA_ARGS__);
#define PrintLog(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_INFO, x, __VA_ARGS__);
#define PrintWarning(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_WARNING, x, __VA_ARGS__);
#define PrintError(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_ERROR, x, __VA_ARGS__);
