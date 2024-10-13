#pragma once
#include "GalaxyAPI.h"

#include <iostream>
#include <string>
#include <cstring>
#include <assert.h>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_LOG_SIZE UINT16_MAX
#define LOG_PATH "Logs/"

namespace GALAXY::Debug
{
    enum class LogType
    {
        L_INFO,
        L_WARNING,
        L_ERROR
    };

    static const char* SerializeLogTypeValue(LogType value)
    {
        switch (value)
        {
        default:
        case LogType::L_INFO:
            return "Info";
        case LogType::L_WARNING:
            return "Warning";
        case LogType::L_ERROR:
            return "Error";
        }
    }

    class GALAXY_API Log
    {
    public:
        ~Log();

        static bool LogToFile;

        static void OpenFile(const std::tm& calendar_time);

        static void WriteToFile(LogType type, const std::tm& calendar_time, char* messageAndFile);

        static void CloseFile();

        template <typename... Args>
        static void Print(const char* file, int line, LogType type, const char* format, Args... args)
        {
            std::time_t now = std::time(nullptr); // get the current time point
            std::tm calendar_time;

            
#ifdef _WIN32
            localtime_s(&calendar_time, &now);
#elif defined(__linux__)
            localtime_r(&now, &calendar_time);
#endif

            if (LogToFile && !m_isFileOpen)
            {
                OpenFile(calendar_time);
            }

            char result[MAX_LOG_SIZE];
            char message[MAX_LOG_SIZE];
            char messageAndFile[MAX_LOG_SIZE];
#ifdef _WIN32
            const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            sprintf_s(message, format, args...);
            sprintf_s(messageAndFile, "%s (l:%d): %s\n", file, line, message);
            sprintf_s(result, "[%02d:%02d:%02d] %s", calendar_time.tm_hour,
                calendar_time.tm_min, calendar_time.tm_sec, messageAndFile);
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
            std::cout << result;
            SetConsoleTextAttribute(hConsole, 15);
#else
			snprintf(message, sizeof(message), format, args...);
            snprintf(messageAndFile, sizeof(messageAndFile), "%s (l:%d): %s\n", file, line, message);
			snprintf(result, sizeof(result), "[%02d:%02d:%02d] %s", calendar_time.tm_hour, 
            calendar_time.tm_min, calendar_time.tm_sec, messageAndFile);
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

			std::cout << result;
			std::cout << "\033[0m"; // Reset console text color
#endif
#ifdef WITH_EDITOR
            AddTextToConsole(type, result);
#endif
            if (LogToFile && m_isFileOpen)
            {
                WriteToFile(type, calendar_time, messageAndFile);
            }
        }

#ifdef WITH_EDITOR
        static void AddTextToConsole(LogType type, const std::string& text);
#endif

    private:
        static bool m_isFileOpen;
        static std::ofstream m_file;
    };
}

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define LOG(t, x, ...) Debug::Log::Print(__FILENAME__, __LINE__, t, x, ##__VA_ARGS__)
#define PrintLog(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_INFO, x, ##__VA_ARGS__)
#define PrintWarning(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_WARNING, x, ##__VA_ARGS__)
#define PrintError(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_ERROR, x, ##__VA_ARGS__)

#define ASSERT(condition) assert(condition)
