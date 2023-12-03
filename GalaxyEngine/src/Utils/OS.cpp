#include "pch.h"
#include "Utils/OS.h"

#include "Core/Application.h"

#include "Editor/UI/FileDialog.h"

#include "Resource/ResourceManager.h"

#ifdef __linux__
#include <dlfcn.h>
#endif

namespace GALAXY
{

	std::string Utils::OS::SaveDialog(const char* filter)
	{
#if defined(_WIN32) && !defined(HANDLE_FILE_DIALOG)
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = Core::Application::GetInstance().GetWindow()->GetWindowWIN32();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		ofn.lpstrInitialDir = Resource::ResourceManager::GetInstance()->GetAssetPath().string().c_str();
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
#elif defined(HANDLE_FILE_DIALOG)
		Editor::UI::FileDialog::OpenFileDialog(Editor::UI::FileDialogType::Save, filter, Resource::ResourceManager::GetInstance()->GetAssetPath());
#endif
		return "";
	}

	std::string Utils::OS::OpenDialog(const char* filter)
	{
#if defined(_WIN32) && !defined(HANDLE_FILE_DIALOG)
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = Core::Application::GetInstance().GetWindow()->GetWindowWIN32();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		ofn.lpstrInitialDir = Resource::ResourceManager::GetInstance()->GetAssetPath().string().c_str();
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			if (ofn.lpstrFile != nullptr)
				return ofn.lpstrFile;
			return "";
		}
#elif defined(HANDLE_FILE_DIALOG)
		Editor::UI::FileDialog::OpenFileDialog(Editor::UI::FileDialogType::Open, filter, Resource::ResourceManager::GetInstance()->GetAssetPath());
#endif
		return "";
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
