#include <Core/Application.h>

using namespace GALAXY;
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// Main code
void Main(int argc, char** argv)
{
	/*
	float arr[] = {
	1, 5, 6, 7,
		8, 9, 9, 10,
		5, 8, 9, 8,
		1, 2, 7, 3
	};
	auto mat1 = Math::Mat4(arr);
	auto mat2 = mat1;
	(mat1 * mat2).Print();
	(mat1.GetTranspose() * mat2.GetTranspose()).Print();
	*/
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