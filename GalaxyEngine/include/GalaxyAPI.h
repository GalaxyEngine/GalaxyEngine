#pragma once
namespace GALAXY {}
using namespace GALAXY;

#if defined(_WIN32) && defined(GALAXY_EXPORTS)
#define GALAXY_API __declspec(dllexport)
#elif defined(_WIN32)
#define GALAXY_API __declspec(dllimport)
#else
#define GALAXY_API
#endif

#ifdef GALAXY_EXPORTS
	#ifdef ENABLE_REFLECTION
		#include <rttr/registration_friend.h>
		#define REFLECTION_FRIEND RTTR_REGISTRATION_FRIEND
	#else
		#define REFLECTION_FRIEND
	#endif
#endif