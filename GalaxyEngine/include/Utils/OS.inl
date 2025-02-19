#pragma once
#include <regex>

#include "Utils/OS.h"

#ifdef __linux__
#include <dlfcn.h>
#endif

namespace GALAXY 
{
	template<typename T>
	inline T Utils::OS::GetDLLMethod(void* dll, const char* name)
	{
#ifdef _WIN32
		return reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(dll), name));
#elif defined(__linux__)
		return reinterpret_cast<T>(dlsym(dll, name));
#endif
		return T();
	}
	
	inline std::filesystem::path Utils::OS::GetEnvVar(std::string const& key)
	{
#ifdef _WIN32
		size_t requiredSize = 0;
		// Get the size of the environment variable's value.
		errno_t err = getenv_s(&requiredSize, nullptr, 0, key.c_str());
		if (err != 0 || requiredSize == 0)
		{
			return std::string("");
		}

		// Allocate a buffer of the appropriate size.
		std::vector<char> buffer(requiredSize);
		err = getenv_s(&requiredSize, buffer.data(), requiredSize, key.c_str());
		if (err != 0)
		{
			return std::string("");
		}

		return std::string(buffer.data());
#else
		char * val = getenv(key.c_str());
		return val == nullptr ? std::string("") : std::string(val);
#endif
	}

}
