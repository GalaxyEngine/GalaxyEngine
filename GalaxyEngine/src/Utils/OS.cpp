#include "pch.h"
#include "Utils/OS.h"

#include "Core/Application.h"

#include "Resource/ResourceManager.h"

#include <nfd.hpp>

namespace GALAXY
{

	std::string Utils::OS::SaveDialog(const std::vector<Filter>& filters)
	{
		std::string resultString = "";

		NFD::Guard nfdGuard;

		NFD::UniquePath outPath;

		const size_t count = filters.size();
		std::vector<nfdfilteritem_t> filterItems(count);

		for (size_t i = 0; i < count; i++)
		{
			filterItems[i].name = filters[i].name.c_str();
			filterItems[i].spec = filters[i].spec.c_str();
		}

		// show the dialog
		const nfdresult_t result = NFD::SaveDialog(outPath, filterItems.data(), static_cast<uint32_t>(count));
		if (result == NFD_OKAY) {
			resultString = std::string(outPath.get());
		}
		else if (result == NFD_CANCEL) {
		}
		else {
		}

		// NFD::Guard will automatically quit NFD.
		return resultString;
	}

	std::string Utils::OS::OpenDialog(const std::vector<Filter>& filters)
	{
		std::string resultString = "";

		// initialize NFD
		NFD::Guard nfdGuard;

		// auto-freeing memory
		NFD::UniquePath outPath;

		const size_t count = filters.size();
		// prepare filters for the dialog
		std::vector<nfdfilteritem_t> filterItems(count);

		for (size_t i = 0; i < count; i++)
		{
			filterItems[i].name = filters[i].name.c_str();
			filterItems[i].spec = filters[i].spec.c_str();
		}

		// show the dialog
		const nfdresult_t result = NFD::OpenDialog(outPath, filterItems.data(), static_cast<uint32_t>(count));
		if (result == NFD_OKAY) {
			resultString = std::string(outPath.get());
		}
		else if (result == NFD_CANCEL) {
		}
		else {
		}

		// NFD::Guard will automatically quit NFD.
		return resultString;
	}

	std::string Utils::OS::GetLastErrorMessage()
	{
		std::string message;
#ifdef _WIN32
		//Get the error message ID, if any.
		const DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0) {
			return {}; //No error message has been recorded
		}

		LPSTR messageBuffer = nullptr;

		//Ask Win32 to give us the string version of that message ID.
		//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
		const size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		//Copy the error message into a std::string.
		message = std::string(messageBuffer, size);

		//Free the Win32's string's buffer.
		LocalFree(messageBuffer);
#elif defined(__linux__)
		message = dlerror();
#endif
		return message;
	}

	void* Utils::OS::LoadDLL(const std::filesystem::path& DllPath)
	{
		void* handle = nullptr;

#if defined(_WIN32)
		handle = LoadLibrary(DllPath.generic_string().c_str());
#elif defined(__linux__)
		handle = dlopen(DllPath.generic_string().c_str(), RTLD_LAZY);
#endif

		if (!handle) {
			const std::string errorMessage = GetLastErrorMessage();
			PrintError("Failed to load DLL %s. Error : %s", DllPath.generic_string().c_str(), errorMessage.c_str());
		}
		return handle;
	}

	void Utils::OS::FreeDLL(void* dll)
	{
		if (!dll)
			return;
#if defined(_WIN32)
		FreeLibrary((HMODULE)dll);
#elif defined(__linux__)
		dlclose(dll);
#endif
	}

	const char* Utils::OS::GetDLLExtension()
	{
#if defined(_WIN32)
		return ".dll";
#elif defined(__linux__)
		return ".so";
#elif defined(__APPLE__)
		return ".dylib";
#endif
	}

	void Utils::OS::OpenWithVSCode(const std::filesystem::path& filePath)
	{
		std::string command = "code ";
		command += "\"" + filePath.string();
		system(command.c_str());
	}

#ifdef _WIN32
	BOOL CALLBACK EnumWindowsProc(const HWND hwnd, const LPARAM lParam) {
		char windowTitle[256];
		GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

		// Check if the window title contains "Microsoft Visual Studio"
		if (strstr(windowTitle, "Microsoft Visual Studio") != nullptr) {
			// Check for additional criteria to identify the specific instance
			if (strstr(windowTitle, Resource::ResourceManager::GetInstance()->GetProjectPath().filename().string().c_str()) != nullptr) {
				// We found the specific instance of Visual Studio
				*reinterpret_cast<HWND*>(lParam) = hwnd;
				return FALSE; // Stop enumerating windows
			}
		}

		return TRUE; // Continue enumerating windows
	}

	void Utils::OS::OpenWithVS(const std::filesystem::path& filePath)
	{
		// Find the Visual Studio window by its class name or window title
		std::string windowName = Resource::ResourceManager::GetInstance()->GetProjectPath().filename().stem().string() + " - Microsoft Visual Studio";

		HWND hwnd = nullptr; // This will hold the window handle of the specific instance

		// Enumerate windows to find the specific instance of Visual Studio
		EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&hwnd));
		if (!hwnd) {
			const auto resourceManager = Resource::ResourceManager::GetInstance();
			std::string slnPath = (resourceManager->GetAssetPath().parent_path() / "vsxmake2022" / (resourceManager->GetProjectPath().filename().stem().string() + ".sln")).string();
			std::string command = "start \"\" \"" + slnPath + "\"";
			system(command.c_str());
		}
		else
		{
			// Visual Studio window exists, bring it to the foreground
			SetForegroundWindow(hwnd);
			SetActiveWindow(hwnd);

			//std::string command = " /edit ";
			//const std::string env = "devenv.exe";
			//const std::string newPath = "\"" + path.string() + "\"";
			//command += newPath;

			//// Open file with the first instance of Visual Studio
			//ShellExecuteA(hwnd, "open", env.c_str(), command.c_str(), NULL, SW_SHOWNORMAL);
		}
	}
#endif

	void Utils::OS::ShowFile(const std::filesystem::path& filePath, bool showFile)
	{
#if defined(_WIN32)
		int attr = GetFileAttributes((LPCTSTR)filePath.string().c_str());
		if (!showFile) {
			if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0) {
				SetFileAttributes((LPCTSTR)filePath.string().c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
			}
		}
		else {
			if ((attr & FILE_ATTRIBUTE_HIDDEN) != 0) {
				SetFileAttributes((LPCTSTR)filePath.string().c_str(), attr & ~FILE_ATTRIBUTE_HIDDEN);
			}
		}
#elif defined(__linux__)
		// Change file permissions to hide or unhide the file
		if (!showFile)
		{
			if (chmod(filePath.string().c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0)
			{
				std::cerr << "Error: Unable to set file permissions.\n";
				return;
			}
		}
		else
		{
			if (chmod(filePath.string().c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) != 0)
			{
				std::cerr << "Error: Unable to set file permissions.\n";
				return;
			}
		}
#endif
	}
}