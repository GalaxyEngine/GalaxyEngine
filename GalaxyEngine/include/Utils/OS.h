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

		// Show or Hide file with the path of the file
		void ShowFile(const std::filesystem::path& filePath, bool showFile = true);

		std::string GetLastErrorMessage();

		void* LoadDLL(const std::filesystem::path& DllPath);

		void FreeDLL(void* dll);

		template<typename T>
		inline T GetDLLMethod(void* dll, const char* name);

		void ShowInExplorer(const std::filesystem::path& folder, const std::vector<std::string>& filesName);

		const char* GetDLLExtension();

		void OpenWithVSCode(const std::filesystem::path& filePath);

#ifdef _WIN32
		void OpenWithVS(const std::filesystem::path& filePath);
#endif
	}
}
#include "Utils/OS.inl" 
