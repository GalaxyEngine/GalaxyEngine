#include <Core/Application.h>

using namespace GALAXY;
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include <cstdlib>
#include <windows.h>


// Main code
void Main(int argc, char** argv)
{
	std::cout << argc << std::endl;
	for (int i = 0; i < argc; i++)
		std::cout << argv[i] << std::endl;

	// Extract the file path from command-line arguments
	const char* filePath = argv[0];
	auto workDir = std::filesystem::path(argv[0]).parent_path();

	// Change the working directory to where you want it to be
	std::filesystem::current_path(workDir);

	Core::Application& application = Core::Application::GetInstance();

	application.Initalize();

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