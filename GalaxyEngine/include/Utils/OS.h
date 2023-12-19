#pragma once
#include "GalaxyAPI.h"

#if defined(_WIN32) && defined(_MSC_VER)
#pragma comment(lib, "Comdlg32.lib")
#elif defined(__linux__)
#define HANDLE_FILE_DIALOG
#endif
#define HANDLE_FILE_DIALOG

namespace GALAXY 
{
	namespace Utils::OS
	{
		struct Filter
		{
			std::string name;
			// ex : "Text file"
			std::string spec;
			// ex : "txt"
		};

		std::string SaveDialog(const std::vector<Filter>& filters);

		std::string OpenDialog(const std::vector<Filter>& filters);

		std::string GetLastErrorMessage();

		void* LoadDLL(const std::filesystem::path& DllPath);

		void FreeDLL(void* dll);

		template<typename T>
		inline T GetDLLMethod(void* dll, const char* name);

		const char* GetDLLExtension();

		void OpenWithVSCode(const std::filesystem::path& filePath);
	}
}
#include "Utils/OS.inl" 
