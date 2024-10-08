#pragma once
namespace GALAXY {}
using namespace GALAXY;

#ifndef IMGUI_IMPLEMENTATION // for galaxy maths lib
	#define IMGUI_IMPLEMENTATION
	#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#if defined(_WIN32)
	#if defined(GALAXY_EXPORTS)
		#define GALAXY_API __declspec(dllexport)
	#else
		#define GALAXY_API __declspec(dllimport)
	#endif // GALAXY_EXPORTS
#elif defined(__linux__)
	#define GALAXY_API __attribute__((visibility("default")))
#endif