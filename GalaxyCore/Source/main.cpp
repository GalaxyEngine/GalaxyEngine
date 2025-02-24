#include <Core/Application.h>

using namespace GALAXY;

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _WIN32


// Main code
void Main(int argc, char** argv, char** envp)
{
	/*
	 * TODO : Fix shader loading and sending
	 * TODO : Handle update of materials for thumbnails
	 * TODO : Fix sub Meshes for fbx models
	 * TODO : Add Line renderer component
	 * TODO : Fix Forward not correct
	 * TODO : Fix rotation Gizmo (To test)
	 * TODO : Implement Shadow Mapping
	 */

	/*
	for (char **env = envp; *env != 0; env++)
	{
		char *thisEnv = *env;
		std::cout << thisEnv << std::endl;
	}
	*/
	
	std::filesystem::path exePath = std::filesystem::path(argv[0]);
	// On development			   							exe/	 mode/        architecture/ platform/	  build/		GalaxyEngine/
	std::filesystem::path workDir = std::filesystem::path(argv[0]).parent_path().parent_path().parent_path().parent_path().parent_path() / "GalaxyCore";

	Core::Application::ExePath = exePath; // the default path
	if (!std::filesystem::exists(workDir))
	{
		// on package
		workDir = exePath.parent_path();
	}
	// const auto workDir = std::filesystem::path(argv[0]).parent_path();
	// Change the working directory
	std::filesystem::current_path(workDir);

	Core::Application& application = Core::Application::GetInstance();
	std::filesystem::path projectPath;

	if (argc > 1)
		projectPath = std::filesystem::path(argv[1]);

	application.Initialize(projectPath);

	application.Update();

	application.Destroy();
}

int main(int argc, char** argv, char** envp)
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//  TODO: Remove Comments To Break on leaks
	// |
	// V
	// _CrtSetBreakAlloc(15393);
#endif
	Main(argc, argv, envp);
	return 0;
}