#include "pch.h"
#include "Resource/Script.h"
#include "Resource/ResourceManager.h"

#include "Editor/EditorSettings.h"

#include "Scripting/ScriptEngine.h"
namespace GALAXY
{
	const char* hFileContent =
		R"(
#include "Scripting/Macro.h"
#include "Component/ScriptComponent.h"
// Note : all ptr variable need to be initialized with nullptr

class %s : public Component::ScriptComponent
{
	GENERATED_BODY(%s, Component::ScriptComponent)
public:

public:
	void OnStart() override
	{

	}

	void OnUpdate() override
	{

	}

};
CLASS(%s)
	)";

	void Resource::Script::Load()
	{
		// do not load .cpp file
		if (p_loaded || this->p_fileInfo.GetExtension() == ".cpp")
			return;
		std::weak_ptr<Script> resource = Resource::ResourceManager::GetInstance()->GetResource<Script>(this->GetFileInfo().GetRelativePath());
		Scripting::ScriptEngine::GetInstance()->AddScript(resource);
		p_loaded = true;

		//OpenScript(GetFileInfo().GetFullPath());
	}

	void Resource::Script::Unload()
	{
		// Remove the resource from ScriptEngine, remove all component when the user rebuild the dll
		std::weak_ptr<Script> resource = Resource::ResourceManager::GetInstance()->GetResource<Script>(this->GetFileInfo().GetRelativePath());
		Scripting::ScriptEngine::GetInstance()->RemoveScript(resource);
		p_loaded = false;
	}

	Weak<Resource::Script> Resource::Script::Create(const Path& path)
	{
		std::ofstream hFile(path.string() + ".h");
		auto className = path.filename();
		char content[512];
		snprintf(content, sizeof(content), hFileContent, className.string().c_str(), className.string().c_str(), className.string().c_str());
		if (hFile.is_open()) {

			hFile << content;
			hFile.close();
		}

		std::ofstream cppFile(path.string() + ".cpp");
		if (cppFile.is_open()) {
			std::string cppContent;
			cppContent = std::string("#include \"") + path.filename().string() + ".h\"";
			cppFile << cppContent;
			cppFile.close();
		}
		Resource::ResourceManager::GetInstance()->GetOrLoad<Script>(path.string() + ".cpp");
		return Resource::ResourceManager::GetInstance()->GetOrLoad<Script>(path.string() + ".h");
	}

	void Resource::Script::OpenScript(const Path& path)
	{
		switch (Editor::EditorSettings::GetInstance().GetExternalTool())
		{
		case Editor::ExternalTool::VisualStudio:
		{
			OpenWithVS(path);
			break;
		}
		case Editor::ExternalTool::VisualStudioCode:
		{
			OpenWithVSCode(path);
			break;
		}
		default:
			break;
		}
	}
	//TODO : Move to os specific file + add linux support

	void Resource::Script::OpenWithVSCode(const Path& path)
	{
#ifdef _WIN32
		std::string command = path.string();
		std::string env = "code";

		ShellExecuteA(NULL, "open", env.c_str(), command.c_str(), NULL, SW_SHOWNORMAL);
#else
		//TODO
#endif
	}

#ifdef _WIN32
	BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
		char windowTitle[256];
		GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

		// Check if the window title contains "Microsoft Visual Studio"
		if (strstr(windowTitle, "Microsoft Visual Studio") != nullptr) {
			// Check for additional criteria to identify the specific instance
			if (strstr(windowTitle, Resource::ResourceManager::GetInstance()->GetProjectPath().filename().string().c_str()) != nullptr) {
				// We found the specific instance of Visual Studio
				*(HWND*)lParam = hwnd;
				return FALSE; // Stop enumerating windows
			}
		}

		return TRUE; // Continue enumerating windows
	}
#endif

	void Resource::Script::OpenWithVS(const Path& path)
	{
#ifdef _WIN32
		// Find the Visual Studio window by its class name or window title
		std::string windowName = Resource::ResourceManager::GetInstance()->GetProjectPath().filename().string() + " - Microsoft Visual Studio";

		HWND hwnd = nullptr; // This will hold the window handle of the specific instance

		// Enumerate windows to find the specific instance of Visual Studio
		EnumWindows(EnumWindowsProc, (LPARAM)&hwnd);
		if (hwnd)
		{
			// Visual Studio window exists, bring it to the foreground
			SetForegroundWindow(hwnd);
			SetActiveWindow(hwnd);

			std::string command = " /edit ";
			std::string env = "devenv.exe";
			std::string newPath = path.string();
			command += newPath;

			// Open file with the first instance of Visual Studio
			ShellExecuteA(hwnd, "open", env.c_str(), command.c_str(), NULL, SW_SHOWNORMAL);
		}
		else
		{
			// No Visual Studio window found, open a new window
			std::string newPath = path.string();
			const std::string commandLineArgs = "\"" + path.string() + "\" /command \"Edit.OpenFile " + newPath + "\"";
			ShellExecuteA(nullptr, "open", "devenv.exe", commandLineArgs.c_str(), nullptr, SW_SHOWNORMAL);
		}
#else
		//TODO
#endif
	}

}
