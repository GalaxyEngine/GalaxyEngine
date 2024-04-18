#pragma once
#include "GalaxyAPI.h"
#include "Debug/Log.h"

namespace GALAXY::Debug
{
    class TimeProfiler
    {
    public:
        explicit TimeProfiler(const char* funcName);
        
        template <typename ...Args>
        TimeProfiler(const char* funcName, const char* format, Args ... args)
        {
            m_funcName = funcName;
            char buf[MAX_LOG_SIZE];
#ifdef _WIN32
            sprintf_s(buf, format, args ...);
#elif defined(__linux__)
			snprintf(buf, sizeof(buf2), format, args...);
#endif
            m_content = std::string(buf);
            m_startTime = std::chrono::steady_clock::now();
        }
        
        ~TimeProfiler();
    private:
        std::chrono::time_point<std::chrono::steady_clock> m_startTime;
        std::string m_funcName;
        std::string m_content;
    };
}

#define PROFILE_SCOPE() GALAXY::Debug::TimeProfiler funcNameProfiler(__FUNCTION__);
#define PROFILE_SCOPE_LOG(x, ...) GALAXY::Debug::TimeProfiler funcNameProfiler(__FUNCTION__, x, ##__VA_ARGS__);
