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
		const std::weak_ptr<Script> resource = Resource::ResourceManager::GetInstance()->GetResource<Script>(this->GetFileInfo().GetRelativePath());
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
		Editor::ScriptEditorTool tool = Editor::EditorSettings::GetInstance().GetScriptEditorTool();
		switch (tool)
		{
#ifdef _WIN32
		case Editor::ScriptEditorTool::VisualStudio:
		{
			OpenWithVS(path);
			break;
		}
#endif
		case Editor::ScriptEditorTool::VisualStudioCode:
		{
			OpenWithVSCode(path);
			break;
		}
		default:
			break;
		}
	}
	//TODO : Move to os specific file

	void Resource::Script::OpenWithVSCode(const Path& path)
	{
		std::string command = "code ";
		command += "\"" + ResourceManager::GetInstance()->GetAssetPath().parent_path().string() + "\"";
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
#endif

	void Resource::Script::OpenWithVS(const Path& path)
	{
#ifdef _WIN32
	// Find the Visual Studio window by its class name or window title
		std::string windowName = Resource::ResourceManager::GetInstance()->GetProjectPath().filename().stem().string() + " - Microsoft Visual Studio";

		HWND hwnd = nullptr; // This will hold the window handle of the specific instance

		// Enumerate windows to find the specific instance of Visual Studio
		EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&hwnd));
		if (!hwnd) {
			const auto resourceManager = ResourceManager::GetInstance();
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
#endif
	}

}
