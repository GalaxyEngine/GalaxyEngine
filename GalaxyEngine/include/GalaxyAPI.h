#pragma once
namespace GALAXY {}
using namespace GALAXY;

#if defined(_WIN32)
	#if defined(GALAXY_EXPORTS)
		#define GALAXY_API __declspec(dllexport)
	#else
		#define GALAXY_API __declspec(dllimport)
	#endif // GALAXY_EXPORTS
#elif defined(__linux__)
	#define GALAXY_API __attribute__((visibility("default")))
#endif

#ifdef GALAXY_EXPORTS
	#ifdef ENABLE_REFLECTION
		#include <rttr/registration_friend.h>
		#define REFLECTION_FRIEND RTTR_REGISTRATION_FRIEND
	#else
		#define REFLECTION_FRIEND
	#endif
#endif