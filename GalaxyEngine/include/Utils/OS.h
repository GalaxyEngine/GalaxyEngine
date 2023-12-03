#pragma once
#include "GalaxyAPI.h"

#if defined(_WIN32) && defined(_MSC_VER)
#pragma comment(lib, "Comdlg32.lib")
#elif defined(__linux__)
#define HANDLE_FILE_DIALOG
#endif

namespace GALAXY 
{
	namespace Utils::OS
	{
		std::string SaveDialog(const char* filter);

		std::string OpenDialog(const char* filter);

		std::string GetLastErrorMessage();

		void* LoadDLL(const std::filesystem::path& DllPath);

		void FreeDLL(void* dll);

		template<typename T>
		inline T GetDLLMethod(void* dll, const char* name);

		const char* GetDLLExtension();
	}
}
#include "Utils/OS.inl" 
