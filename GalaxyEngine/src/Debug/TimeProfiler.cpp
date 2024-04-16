#include "pch.h"
#include "Debug/TimeProfiler.h"
namespace GALAXY 
{
    Debug::TimeProfiler::TimeProfiler(const char* funcName) : m_funcName(funcName)
    {
        m_startTime = std::chrono::steady_clock::now();
    }

    Debug::TimeProfiler::~TimeProfiler()
    {
        if (m_funcName.length() > 0)
            PrintLog("Function %s took %f ms : %s", m_funcName.c_str(), std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_startTime).count(), m_content.c_str());
        else
            PrintLog("Function %s took %f ms", m_funcName.c_str(), std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_startTime).count());
    }
}
