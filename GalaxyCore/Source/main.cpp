#include <Core/Application.h>

using namespace GALAXY;

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _WIN32


// Main code
void Main(int argc, char** argv)
{
	//TODO : Fix shader loading and sending
	//TODO : Handle update of materials for thumbnails
	//TODO : Fix sub Meshes for fbx models
	//TODO : Add packaging project
	//TODO : Add Line renderer component

	const auto workDir = std::filesystem::path(argv[0]).parent_path();
	// Change the working directory
	std::filesystem::current_path(workDir);

	Core::Application& application = Core::Application::GetInstance();
	
	//TODO : Remove this
	std::filesystem::path projectPath;
#ifdef _WIN32
	projectPath = "D:/Code/Moteurs/Galaxy Projects/GameTest/GameTest.gProject";
	if (!std::filesystem::exists(projectPath))
		projectPath = "C:/Users/romai/Documents/Code/Projects Galaxy/ProjectA/ProjectA.gProject";
#elif defined(__linux__)
	projectPath = "/home/uwu/Documents/GalaxyProject/GalaxyProject.gProject";
#endif
	//std::filesystem::path projectPath = "D:/Code/Test Projects/Project/Project.gProject";
#ifdef WITH_GAME
	// find a dll in the current folder
	for (const auto& entry : std::filesystem::directory_iterator("."))
	{
		std::filesystem::path filename = entry.path().filename();
		bool isADll = false;
#if defined(_WIN32)
		isADll = filename.extension() == ".dll";
#elif defined(__linux__)
		isADll = filename.extension() == ".so";
#elif defined(__APPLE__)
		isADll = filename.extension() == ".dylib";
#endif
		if (isADll)
		{
			const bool isEngineDll = filename == "GalaxyGame.dll" || filename == "GalaxyGameDebug.dll" || filename == "GalaxyEditor.dll" || filename == "GalaxyEditorDebug.dll";
			if (isEngineDll)
				continue;
			projectPath = entry.path();
			projectPath = (projectPath.parent_path() / projectPath.filename().stem()).generic_string() + ".gProject";
			break;
		}
	}
#endif
	if (argc > 1)
		projectPath = std::filesystem::path(argv[1]);

	application.Initialize(projectPath);

	application.Update();

	application.Destroy();
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//  TODO: Remove Comments To Break on leaks
	// |
	// V
	//_CrtSetBreakAlloc(863);
#endif
	Main(argc, argv);
	return 0;
}