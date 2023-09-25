#include <Core/Application.h>

using namespace GALAXY;
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include <cstdlib>
#include <windows.h>


// Main code
void Main(int argc, char** argv)
{
	auto workDir = std::filesystem::path(argv[0]).parent_path();
	// Change the working directory
	std::filesystem::current_path(workDir);

	Core::Application& application = Core::Application::GetInstance();

	std::filesystem::path projectPath/* = "D:\\Code\\ProjectTest"*/;
	if (argc > 1)
		projectPath = std::filesystem::path(argv[1]).parent_path();

	application.Initalize(projectPath);

	application.Update();

	application.Destroy();

}

int main(int argc, char** argv)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//  TODO: Remove Comments To Break on leaks
	// |
	// V
	//_CrtSetBreakAlloc(799);
	Main(argc, argv);
	return 0;
}