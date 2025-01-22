#pragma once
namespace GALAXY {}
using namespace GALAXY;

#ifndef IMGUI_IMPLEMENTATION // for galaxy maths lib
	#define IMGUI_IMPLEMENTATION
	#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#ifdef WITH_PROJECT
	#if defined(_MSC_VER)  // MSVC
		#define EDITOR_ONLY __declspec(deprecated("This method is editor only."))
	#elif defined(__GNUC__) || defined(__clang__)  // GCC, MinGW, or Clang
		#define EDITOR_ONLY __attribute__((deprecated("This method is editor only.")))
	#else if 
		#define EDITOR_ONLY
	#endif
#else
	#define EDITOR_ONLY
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