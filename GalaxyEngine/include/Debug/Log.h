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
    // Helper function to format a string using printf-style syntax
    template <typename... Args>
    static std::string FormatString(const char* format, Args... args)
    {
        int size = std::snprintf(nullptr, 0, format, args...);
        if (size < 0) return "";
        std::string buffer(size + 1, '\0');
        std::snprintf(&buffer[0], buffer.size(), format, args...);
        buffer.resize(size); // Remove null terminator
        return buffer;
    }
    
    enum class LogType
    {
        L_INFO,
        L_WARNING,
        L_ERROR
    };

    inline const char* SerializeLogTypeValue(LogType value)
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

        static void WriteToFile(LogType type, const std::tm& calendar_time, const char* messageAndFile);

        static void CloseFile();

        template <typename... Args>
        static void Print(const char* file, int line, LogType type, const char* format, Args... args)
        {
            std::time_t now = std::time(nullptr);
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

            std::string message = FormatString(format, args...);
            std::string fileLine = FormatString("%s (l:%d): ", file, line);
            std::string messageAndFile = fileLine + message + "\n";
            std::string header = FormatString("[%02d:%02d:%02d] ", calendar_time.tm_hour, calendar_time.tm_min, calendar_time.tm_sec);
            std::string result = header + messageAndFile;

        #ifdef _WIN32
            const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, 10); // Green for header
            std::cout << header;
            SetConsoleTextAttribute(hConsole, 14); // Yellow for fileLine
            std::cout << fileLine;
            switch (type) {
                case LogType::L_INFO:
                    SetConsoleTextAttribute(hConsole, 15); // White
                    break;
                case LogType::L_WARNING:
                    SetConsoleTextAttribute(hConsole, 14); // Yellow
                    break;
                case LogType::L_ERROR:
                    SetConsoleTextAttribute(hConsole, 4); // Red
                    break;
                default:
                    break;
            }
            std::cout << message << "\n";
            SetConsoleTextAttribute(hConsole, 15); // Reset to default
        #else
            switch (type)
            {
                case LogType::L_INFO:
                    std::cout << "\033[37m"; // White
                    break;
                case LogType::L_WARNING:
                    std::cout << "\033[33m"; // Yellow
                    break;
                case LogType::L_ERROR:
                    std::cout << "\033[31m"; // Red
                    break;
                default:
                    break;
            }
            std::cout << result;
            std::cout << "\033[0m"; // Reset color
        #endif

        #ifdef WITH_EDITOR
            AddTextToConsole(type, result.c_str());
        #endif

            if (LogToFile && m_isFileOpen)
            {
                WriteToFile(type, calendar_time, messageAndFile.c_str());
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

#define ASSERT(x) if (!(x)) { PrintError("Assertion failed: %s", #x); assert(false); }
