#include "pch.h"
#include "Utils/OS.h"

#include "Core/Application.h"
#include "Core/ThreadManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture.h"

#include <nfd.hpp>
#include <iostream>
#include <array>
#include <cstdio>
#include <memory>

#include "Render/Framebuffer.h"
#include "Wrapper/ImageLoader.h"

#ifdef __linux__
#include <sys/stat.h>
#elif defined(_WIN32)
#include <shlobj.h>
#endif

namespace GALAXY
{
	std::filesystem::path GALAXY::Utils::OS::GetUserAppDataFolder()
	{
		std::filesystem::path result;
#ifdef _WIN32
		char path[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path)))
		{
			result = path;
		}
		else
		{
			PrintError("Failed to get AppData folder path");
		}
#elif defined(__linux__)
		const char* homedir = getenv("HOME");
		if (homedir == NULL)
		{
			PrintError("Failed to get Home directory folder path");
		}
		else
		{
			result = homedir;
			result /= ".local/share/";
		}
#endif // _WIN32

		return result;
	}

	std::string Utils::OS::SaveDialog(const std::vector<Filter>& filters)
	{
		std::string resultString;

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
		std::string resultString;

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
			nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);

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
		FreeLibrary(static_cast<HMODULE>(dll));
#elif defined(__linux__)
		dlclose(dll);
#endif
	}
	
	void Utils::OS::ShowInExplorer(const std::filesystem::path& folder, const std::vector<std::string>& filesName)
	{
#ifdef _WIN32
		const bool select = !filesName.empty();

		const char* explorerPath = "explorer.exe";

		// Construct the command
		const char* command = select ? "/select,\"" : "\"";
		char fullCommand[MAX_PATH + sizeof(command) + 2];

		std::string file;
		if (select)
			file = (folder / filesName.back()).string();
		else
			file = folder.string();
		snprintf(fullCommand, sizeof(fullCommand), "%s%s\"", command, file.c_str());

		// Launch File Explorer
		HINSTANCE result = ShellExecute(nullptr, "open", explorerPath, fullCommand, nullptr, SW_SHOWNORMAL);
		if (reinterpret_cast<intptr_t>(result) <= 32) {
			const std::string errorMessage = GetLastErrorMessage();
			PrintError("Failed to Open Explorer : %s", errorMessage.c_str());
		}
#elif defined(__linux__)
		std::string command = "xdg-open ";
		std::string fullCommand = command + folder.generic_string() + "/";
		if (std::system(fullCommand.c_str()) != 0) {
			std::perror("Failed to open file explorer");
			// Handle error as needed
		}
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
		std::system(command.c_str());
	}

	void Utils::OS::OpenWith(const std::filesystem::path& appPath, const std::filesystem::path& filePath)
	{
		auto prevPath = std::filesystem::current_path();
		std::string command = appPath.string();
		command += " \"";
		command += filePath.string();
		std::system(command.c_str());
	}

	void Utils::OS::RunCommand(const std::string& command) {
		// Open a pipe to read the command's output
		std::array<char, 1024> buffer;
		std::string result;
		
		std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);

		if (!pipe) {
			PrintError("popen() failed!");
			return;
		}

		// Read the output a line at a time
		while (fgets(buffer.data(), (int)buffer.size(), pipe.get()) != nullptr) {
			result += buffer.data();
		}

		// Print the result
		PrintLog(result.c_str());
	}

	void Utils::OS::RunCommandThread(const std::string& command)
	{
		Core::ThreadManager::GetInstance()->AddTask(([command] { RunCommand(command); }));
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
		std::string windowName = Resource::ResourceManager::GetProjectPath().filename().stem().string() + " - Microsoft Visual Studio";

		HWND hwnd = nullptr; // This will hold the window handle of the specific instance

		// Enumerate windows to find the specific instance of Visual Studio
		EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&hwnd));
		if (!hwnd) {
			const std::string slnPath = (Resource::ResourceManager::GetAssetPath().parent_path() / "vsxmake2022" / (Resource::ResourceManager::GetProjectPath().filename().stem().string() + ".sln")).string();
			const std::string command = "start \"\" \"" + slnPath + "\"";
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

	void Utils::OS::OpenWithRider(const std::filesystem::path& filePath)
	{
		auto prevPath = std::filesystem::current_path();
		std::string riderPath = "rider";
		const std::string slnPath = (Resource::ResourceManager::GetProjectPath().filename().stem().string() + ".sln");
		std::filesystem::current_path(Resource::ResourceManager::GetAssetPath().parent_path() / "vsxmake2022");
		const std::string command = riderPath + " \"" + slnPath + "\"";
		std::system(command.c_str());
		std::filesystem::current_path(prevPath);
	}

	// Global variables
	HBITMAP hBitmap = NULL;
	int imgWidth, imgHeight; // Store image dimensions

	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			if (hBitmap) {
				HDC hdcMem = CreateCompatibleDC(hdc);
				HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

				// Get window size
				RECT rect;
				GetClientRect(hwnd, &rect);
				int winWidth = rect.right - rect.left;
				int winHeight = rect.bottom - rect.top;

				// Calculate destination rectangle based on image aspect ratio
				float imgAspect = (float)imgWidth / (float)imgHeight;
				int drawWidth, drawHeight;

				if (winWidth / (float)winHeight > imgAspect) {
					drawHeight = winHeight;
					drawWidth = (int)(winHeight * imgAspect);
				}
				else {
					drawWidth = winWidth;
					drawHeight = (int)(winWidth / imgAspect);
				}

				int xOffset = (winWidth - drawWidth) / 2;
				int yOffset = (winHeight - drawHeight) / 2;

				// Stretch the image to fit within the window while maintaining the aspect ratio
				StretchBlt(hdc, xOffset, yOffset, drawWidth, drawHeight, hdcMem, 0, 0, imgWidth, imgHeight, SRCCOPY);

				SelectObject(hdcMem, hbmOld);
				DeleteDC(hdcMem);
			}
			EndPaint(hwnd, &ps);
		}
					 break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}

	void Utils::OS::DisplayImageInPopup(const Wrapper::Image& image, int windowWidth, int windowHeight)
	{
		auto width = image.size.x;
		auto height = image.size.y;
		auto data = image.data;
		// Store image dimensions globally
		imgWidth = width;
		imgHeight = height;

		// Convert RGBA to BGRA
		for (int i = 0; i < width * height; ++i) {
			unsigned char* pixel = data + i * 4;
			std::swap(pixel[0], pixel[2]); // Swap red (R) and blue (B) channels
		}

		// Create the bitmap
		HDC hdc = GetDC(NULL);
		hBitmap = CreateCompatibleBitmap(hdc, width, height);
		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

		BITMAPINFO bmi;
		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = width;
		bmi.bmiHeader.biHeight = -height; // top-down
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		SetDIBits(hdcMem, hBitmap, 0, height, data, &bmi, DIB_RGB_COLORS);

		SelectObject(hdcMem, hbmOld);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdc);

		// Register the window class
		LPCSTR CLASS_NAME = LPCSTR("ImagePopupWindow");
		WNDCLASS wc = { };
		wc.lpfnWndProc = WndProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = CLASS_NAME;
		RegisterClass(&wc);

		// Create the window
		HWND hwnd = CreateWindowEx(
			0,
			CLASS_NAME,
			"Image Popup",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL
		);

		if (!hwnd) {
			MessageBox(NULL, "Failed to create window", "Error", MB_ICONERROR);
			return;
		}

		ShowWindow(hwnd, SW_SHOW);

		// Run the message loop
		MSG msg = { };
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Clean up
		if (hBitmap) {
			DeleteObject(hBitmap);
			hBitmap = NULL;
		}
	}
#elif defined(__linux__)
	
	void Utils::OS::DisplayImageInPopup(const Wrapper::Image& image, int windowWidth, int windowHeight) { PrintError("Not implemented yet"); }
	
#endif // _WIN32
	
	void Utils::OS::DisplayImageInPopup(const std::filesystem::path& imagePath, int windowWidth, int windowHeight)
	{
		auto image = Wrapper::ImageLoader::Load(imagePath);
		DisplayImageInPopup(image, windowWidth, windowHeight);
		Wrapper::ImageLoader::ImageFree(image);
	}

	void Utils::OS::DisplayImageInPopup(Render::Framebuffer* framebuffer, int windowWidth, int windowHeight)
	{
		auto renderer = Wrapper::Renderer::GetInstance();
		Wrapper::Image imageData;
		imageData.size = framebuffer->GetSize();
		imageData.data = new uint8_t[imageData.size.x * imageData.size.y * 4];
		
		renderer->BindRenderBuffer(framebuffer);
		renderer->ReadPixels(imageData.size, imageData.data);
		renderer->UnbindRenderBuffer(framebuffer);
		DisplayImageInPopup(imageData, windowWidth, windowHeight);
	}

	void Utils::OS::DisplayImageInPopup(Resource::Texture* texture, int windowWidth, int windowHeight)
	{
		auto image = Wrapper::ImageLoader::FromTextureToImage(texture);
		DisplayImageInPopup(image, windowWidth, windowHeight);
		Wrapper::ImageLoader::ImageFree(image);
	}

	void Utils::OS::ShowFile(const std::filesystem::path& filePath, bool showFile)
	{
		if (!std::filesystem::exists(filePath)) {
			return;
		}

#if defined(_WIN32)
		const int attr = GetFileAttributes((LPCTSTR)filePath.string().c_str());
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
		// Change file permissions to hide or unhide the 
		//TODO: Test on linux
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