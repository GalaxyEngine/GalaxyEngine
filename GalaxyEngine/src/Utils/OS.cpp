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
		const nfdresult_t result = NFD::SaveDialog(outPath, filterItems.data(), count);
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
		const nfdresult_t result = NFD::OpenDialog(outPath, filterItems.data(), count);
		if (result == NFD_OKAY) {
			resultString = std::string(outPath.get());
			std::cout << "Success!" << std::endl << outPath.get() << std::endl;
		}
		else if (result == NFD_CANCEL) {
			std::cout << "User pressed cancel." << std::endl;
		}
		else {
			std::cout << "Error: " << NFD::GetError() << std::endl;
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

	}
