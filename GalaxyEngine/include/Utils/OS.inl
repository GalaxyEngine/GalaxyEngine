#pragma once
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


}
