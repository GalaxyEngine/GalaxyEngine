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
#include <regex>

#include "Render/Framebuffer.h"
#include "Scripting/ScriptEngine.h"
#include "Utils/Time.h"
#include "Wrapper/ImageLoader.h"

#ifdef __linux__
#include <sys/stat.h>
#elif defined(_WIN32)
#include <shlobj.h>
#endif

namespace GALAXY
{    
#ifdef _WIN32

    struct VSAppInfo {
        std::string displayName;
        std::string version;
        std::string productPath;
        std::string installationPath;  // Example of an extra field
    };

    using VSAppInfos = std::vector<VSAppInfo>;

    VSAppInfos ParseVSWhereResult(const std::string& input) {
        VSAppInfos apps;
        
        // Fields to search for
        std::vector<std::string> fields = { "installationVersion", "productPath",
    	    "displayName", "installationPath" };

        // Regex pattern for key-value pairs
        std::regex fieldRe(R"((\w+):\s*([^\r\n]+))");
        
        // Find each instance block by detecting "instanceId:" as a separator
        std::regex instanceRe(R"(instanceId:\s*[^\r\n]+)");
        auto instanceStart = std::sregex_iterator(input.begin(), input.end(), instanceRe);
        auto instanceEnd = std::sregex_iterator();

        for (auto it = instanceStart; it != instanceEnd; ++it) {
            // Get the substring for this instance
            auto startPos = it->position();
            auto nextPos = (std::next(it) != instanceEnd) ? std::next(it)->position() : input.length();
            std::string instanceBlock = input.substr(startPos, nextPos - startPos);
            
            // Extract fields
            std::unordered_map<std::string, std::string> fieldValues;
            std::sregex_iterator fieldStart(instanceBlock.begin(), instanceBlock.end(), fieldRe);
            std::sregex_iterator fieldEnd;
            
            for (auto fit = fieldStart; fit != fieldEnd; ++fit) {
                fieldValues[(*fit)[1].str()] = (*fit)[2].str();
            }

            // Ensure required fields exist before adding to the result
            if (fieldValues.count("displayName") && fieldValues.count("installationVersion") && fieldValues.count("productPath")) {
                apps.emplace_back(VSAppInfo{
                    fieldValues["displayName"],
                    fieldValues["installationVersion"],
                    fieldValues["productPath"],
                    fieldValues.count("installationPath") ? fieldValues["installationPath"] : ""  // Optional field
                });
            }
        }
        return apps;
    }
    
	std::filesystem::path Utils::FindTool::FindVS()
	{
		std::filesystem::path result;

		std::filesystem::path vsPath = OS::GetEnvVar("VSINSTALLDIR");
		if (vsPath.empty())
		{
			vsPath = OS::GetEnvVar("ProgramFiles(x86)");
			if (vsPath.empty())
			{
				vsPath = OS::GetEnvVar("ProgramFiles");
			}
			if (!vsPath.empty())
			{
				vsPath /= "Microsoft Visual Studio\\Installer";
			}
		}
		
		if (vsPath.empty())
		{
			return result;
		}

		std::string command = "cmd /C \"" + vsPath.string() + "/vswhere.exe\" -prerelease" ;
		std::string output = OS::RunCommand(command, false);

		VSAppInfos apps = ParseVSWhereResult(output);

		//Choose the latest version of Visual Studio
		std::ranges::sort(apps, [](const VSAppInfo& a, const VSAppInfo& b) {
			return a.version > b.version;
		});

		if (!apps.empty())
		{
			result = apps[0].productPath;
		}
		
		return result;
	}
    
    // Helper function: Enumerate versioned subkeys (ignoring "vAny") and retrieve the "installDir" value.
    inline bool GetRiderInstallLocation(HKEY hKeyRoot, std::string& installLocation)
    {
        HKEY hKey;
        const char* baseSubKeyPath = "SOFTWARE\\JetBrains\\Rider";
        // Open the base key. Use KEY_WOW64_64KEY for the 64-bit registry view.
        LONG result = RegOpenKeyExA(hKeyRoot, baseSubKeyPath, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
        if (result != ERROR_SUCCESS) {
            return false;
        }

        bool found = false;
        DWORD index = 0;
        char subKeyName[256];
        DWORD subKeyNameSize = sizeof(subKeyName);
        
        // Enumerate all subkeys under "SOFTWARE\\JetBrains\\Rider"
        while (RegEnumKeyExA(hKey, index, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
        {
            // Skip the "vAny" key, which does not contain an installDir value.
            if (_stricmp(subKeyName, "vAny") == 0) {
                index++;
                subKeyNameSize = sizeof(subKeyName);
                continue;
            }
            
            // Check if the subkey name starts with 'V' (e.g., "V2022.1", "V2023.2", etc.)
            if (subKeyName[0] == 'V' || subKeyName[0] == 'v')
            {
                HKEY hSubKey;
                // Open the version-specific subkey.
                result = RegOpenKeyExA(hKey, subKeyName, 0, KEY_READ | KEY_WOW64_64KEY, &hSubKey);
                if (result == ERROR_SUCCESS)
                {
                    char pathBuffer[MAX_PATH];
                    DWORD bufferSize = sizeof(pathBuffer);
                    // Attempt to retrieve the "installDir" value from the subkey.
                    result = RegQueryValueExA(hSubKey, "installDir", nullptr, nullptr, reinterpret_cast<LPBYTE>(pathBuffer), &bufferSize);
                    RegCloseKey(hSubKey);
                    if (result == ERROR_SUCCESS)
                    {
                        installLocation = std::string(pathBuffer);
                        found = true;
                        break; // Found the installation directory, so exit the loop.
                    }
                }
            }
            index++;
            subKeyNameSize = sizeof(subKeyName); // Reset size for the next iteration.
        }
        RegCloseKey(hKey);
        return found;
    }

    // Retrieves the full path to the Rider executable by checking both HKCU and HKLM.
    inline std::string GetRiderExePath()
    {
        std::string installLocation;

        // First, try looking under HKEY_CURRENT_USER.
        if (!GetRiderInstallLocation(HKEY_CURRENT_USER, installLocation))
        {
            // If not found, try under HKEY_LOCAL_MACHINE.
            if (!GetRiderInstallLocation(HKEY_LOCAL_MACHINE, installLocation))
            {
                // Search in the PATH env
                auto envVar = Utils::OS::GetEnvVar("PATH");
                std::vector paths  = Utils::OS::SplitEnvVar(envVar);

                for (const std::filesystem::path& dir : paths)
                {
                    std::filesystem::path candidate = dir / "rider64.exe";
                    if (std::filesystem::exists(candidate))
                    {
                        return (dir / "rider64.exe").generic_string();
                    }
                }
            }
        }
        
        // Construct the full path to the Rider executable.
        std::string exePath = installLocation + "\\bin\\rider64.exe";
        return exePath;
    }

	std::filesystem::path Utils::FindTool::FindRider()
	{
		return GetRiderExePath();
	}
#endif
    
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

    std::filesystem::path Utils::OS::GetEngineDataFolder()
    {
        return GetUserAppDataFolder() / "GalaxyEngine/";
    }

    int Utils::OS::Snprintf(char* buffer, size_t size, const char* format, ...)
    {
        va_list args;
        va_start(args, format);

#ifdef _WIN32
        int result = vsprintf_s(buffer, size, format, args);
#else
		int result = vsnprintf(buffer, size, format, args);
#endif

        va_end(args);
        return result;
    }

    std::string Utils::OS::SaveDialog(const std::vector<Filter>& filters, const std::filesystem::path& defaultOpenPath)
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
        const nfdresult_t result = NFD::SaveDialog(outPath, filterItems.data(), static_cast<uint32_t>(count), defaultOpenPath.generic_string().c_str());
        if (result == NFD_OKAY)
        {
            resultString = std::string(outPath.get());
        }
        else if (result == NFD_CANCEL)
        {
        }
        else
        {
        }

        // NFD::Guard will automatically quit NFD.
        return resultString;
    }

    std::string Utils::OS::OpenDialog(const std::vector<Filter>& filters, const std::filesystem::path& defaultOpenPath)
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

        const nfdresult_t result = NFD::OpenDialog(outPath, filterItems.data(), static_cast<uint32_t>(count), defaultOpenPath.generic_string().c_str());
        if (result == NFD_OKAY)
        {
            resultString = std::string(outPath.get());
        }
        else if (result == NFD_CANCEL)
        {
        }
        else
        {
        }

        // NFD::Guard will automatically quit NFD.
        return resultString;
    }

    std::string Utils::OS::OpenFolderDialog()
    {
        // auto-freeing memory
        NFD::UniquePath outPath;
        
        std::string resultString;
        const nfdresult_t result = NFD::PickFolder(outPath);
        if (result == NFD_OKAY) {
            resultString = std::string(outPath.get());
        }
        else if (result == NFD_CANCEL) {
        }
        else {
        }

        return resultString;
    }

    std::string Utils::OS::GetLastErrorMessage()
    {
        std::string message;
#ifdef _WIN32
        //Get the error message ID, if any.
        const DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0)
        {
            return {}; //No error message has been recorded
        }

        LPSTR messageBuffer = nullptr;

        //Ask Win32 to give us the string version of that message ID.
        //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
        const size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
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

        if (!handle)
        {
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
        if (reinterpret_cast<intptr_t>(result) <= 32)
        {
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
        return DLL_EXT;
    }

    const char* Utils::OS::GetBinaryExtension()
    {
        return BIN_EXT;
    }
    
    struct FindWindowData {
        FindWindowData(std::string title) : title(std::move(title)) {}
        std::string title  = "";  
        void*       handle = nullptr;
        bool        found  = false;         
    };

#ifdef _WIN32
    BOOL CALLBACK FindWindowProc(HWND hwnd, LPARAM lParam)
    {
        FindWindowData* data = reinterpret_cast<FindWindowData*>(lParam);
        char windowTitle[256];

        if (GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle)) > 0)
        {
            if (std::string(windowTitle).find(data->title) != std::string::npos)
            {
                data->handle = hwnd;
                data->found = true;
                return FALSE;
            }
        }

        return TRUE; // Continue enumerating windows.
    }
#endif

    bool Utils::OS::IsWindowOpen(const std::string& windowTitle)
    {
#ifdef _WIN32
        FindWindowData data{ windowTitle };

        EnumWindows(FindWindowProc, reinterpret_cast<LPARAM>(&data));

        return data.found;
#else
        ASSERT(false && "Not Implemented yet");
        return false;
#endif
    }

    void* Utils::OS::GetWindow(const std::string& windowTitle)
    {
#ifdef _WIN32
        FindWindowData data{ windowTitle };

        EnumWindows(FindWindowProc, reinterpret_cast<LPARAM>(&data));

        return data.handle;
#else
        ASSERT(false && "Not Implemented yet");
        return nullptr;
#endif
    }
    
#ifdef WITH_EDITOR
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
#ifdef _WIN32

    void Utils::OS::OpenWithVS(const std::filesystem::path& filePath)
    {
        Path editorToolPath = Editor::EditorSettings::GetInstance().GetCurrentScriptEditorToolPath();
        std::string command = "cd " + editorToolPath.parent_path().generic_string();
        command += " && " + editorToolPath.filename().generic_string() + " ";
        const std::string slnPath = "\"" + Scripting::ScriptEngine::GetSLNPath().generic_string() + "\"";
        command += slnPath;
        std::string windowName = Resource::ResourceManager::GetProjectPath().filename().stem().string() +
            " - Microsoft Visual Studio";
        if (!IsWindowOpen(windowName))
        {
            ShellExecute(nullptr, "open", editorToolPath.generic_string().c_str(), slnPath.c_str(), nullptr, SW_SHOWNORMAL);
			
            Utils::ElapsedTimer timer;
            while (!IsWindowOpen(windowName))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                double elapsedTime = timer.GetElapsedTime().AsSeconds();
                if (elapsedTime > 5.0f)
                    return;
            }
        }
    
        HWND hwnd = static_cast<HWND>(GetWindow(windowName));
        if (hwnd)
        {
            // Visual Studio window exists, bring it to the foreground
            SetForegroundWindow(hwnd);
            SetActiveWindow(hwnd);

            std::string command = " /edit ";
            const std::string env = editorToolPath.generic_string() ;
            const std::string newPath = "\"" + filePath.string() + "\"";
            command += newPath;

            // Open file with the first instance of Visual Studio
            ShellExecuteA(hwnd, "open", env.c_str(), command.c_str(), NULL, SW_SHOWNORMAL);
        }
    }

    void Utils::OS::OpenWithRider(const std::filesystem::path& filePath)
    {
        Path editorToolPath = Editor::EditorSettings::GetInstance().GetCurrentScriptEditorToolPath();
        const std::string slnPath = "\"" + Scripting::ScriptEngine::GetSLNPath().generic_string() + "\"";
        
        std::string command = slnPath;
        command += " ";
        const std::string env = editorToolPath.generic_string();
        if (!filePath.empty())
        {
            const std::string newPath = "\"" + filePath.string() + "\"";
            command += newPath;
        }
        // Open file with the first instance of Rider
        ShellExecuteA(nullptr, "open", env.c_str(), command.c_str(), NULL, SW_SHOWNORMAL);
    }
#endif
#endif

    static std::string RemoveColorCodes(const std::string& input) {
        // Regular expression to match ANSI color codes (like \x1b[32;1m)
        std::regex colorRegex("\x1b\\[[0-9;]*m");
        return std::regex_replace(input, colorRegex, "");
    }

    std::string Utils::OS::RunCommand(const std::string& command, bool print /* = true*/)
    {
        // Open a pipe to read the command's output
        std::array<char, MAX_LOG_SIZE> buffer;
        std::string result;

#ifdef _WIN32
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
#elif defined(__linux__)
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
#endif

        if (!pipe)
        {
            PrintError("popen() failed!");
            return "";
        }

        // Read the output a line at a time
        while (fgets(buffer.data(), (int)buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        result = RemoveColorCodes(result);

        // Print the result
        if (print)
            PrintLog(result.c_str());
        return result;
    }

    void Utils::OS::RunCommandThread(const std::string& command)
    {
        Core::ThreadManager::GetInstance()->AddTask(([command] { RunCommand(command); }));
    }

#ifdef _WIN32
    // Global variables
    HBITMAP hBitmap = NULL;
    int imgWidth, imgHeight; // Store image dimensions

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                if (hBitmap)
                {
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

                    if (winWidth / (float)winHeight > imgAspect)
                    {
                        drawHeight = winHeight;
                        drawWidth = (int)(winHeight * imgAspect);
                    }
                    else
                    {
                        drawWidth = winWidth;
                        drawHeight = (int)(winWidth / imgAspect);
                    }

                    int xOffset = (winWidth - drawWidth) / 2;
                    int yOffset = (winHeight - drawHeight) / 2;

                    // Stretch the image to fit within the window while maintaining the aspect ratio
                    StretchBlt(hdc, xOffset, yOffset, drawWidth, drawHeight, hdcMem, 0, 0, imgWidth, imgHeight,
                               SRCCOPY);

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
        for (int i = 0; i < width * height; ++i)
        {
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
        WNDCLASS wc = {};
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

        if (!hwnd)
        {
            MessageBox(NULL, "Failed to create window", "Error", MB_ICONERROR);
            return;
        }

        ShowWindow(hwnd, SW_SHOW);

        // Run the message loop
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Clean up
        if (hBitmap)
        {
            DeleteObject(hBitmap);
            hBitmap = NULL;
        }
    }
#elif defined(__linux__)
	
    void Utils::OS::DisplayImageInPopup(const Wrapper::Image& image, int windowWidth, int windowHeight) { ASSERT(false && "Not implemented yet"); }
	
#endif // _WIN32

    Utils::OS::Platform Utils::OS::GetPlatform()
    {
#ifdef _WIN32
        return Platform::Windows;
#elif defined(__linux__)
		return Platform::Linux;
#elif defined(__APPLE__)
		return Platform::MacOS;
#else
		return Platform::Undefined;
#endif
    }

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
        if (!std::filesystem::exists(filePath))
        {
            return;
        }

#if defined(_WIN32)
        const int attr = GetFileAttributes((LPCTSTR)filePath.string().c_str());
        if (!showFile)
        {
            if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0)
            {
                SetFileAttributes((LPCTSTR)filePath.string().c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
            }
        }
        else
        {
            if ((attr & FILE_ATTRIBUTE_HIDDEN) != 0)
            {
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
