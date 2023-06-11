#include "pch.h"
#include <Core/Application.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// Main code
void Main(int argc, char** argv)
{
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
	//_CrtSetBreakAlloc(79);
	Main(argc, argv);
	return 0;
}