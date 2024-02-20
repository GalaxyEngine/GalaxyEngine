#include <Core/Application.h>

using namespace GALAXY;

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _WIN32


// Main code
void Main(int argc, char** argv)
{
	/*TODO :
	 * Implement GameObject and Component on Scripting
	*/
	const auto workDir = std::filesystem::path(argv[0]).parent_path();
	// Change the working directory
	std::filesystem::current_path(workDir);

	Core::Application& application = Core::Application::GetInstance();
	
	//TODO : Remove this
	#ifdef _WIN32
#if 1
	std::filesystem::path projectPath = "D:/Code/Moteurs/Galaxy Projects/GalaxyGame/GalaxyGame.gProject";
#else
	std::filesystem::path projectPath = "D:/Code/Projects Galaxy/ProjectA/ProjectA.gProject";
#endif
	#elif defined(__linux__)
	std::filesystem::path projectPath = "/home/uwu/Documents/ProjectTest/ProjectTest.gProject";
	#endif
	//std::filesystem::path projectPath = "D:/Code/Test Projects/Project/Project.gProject";
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