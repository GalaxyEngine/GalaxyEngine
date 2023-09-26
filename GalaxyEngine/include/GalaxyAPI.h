#pragma once
namespace GALAXY {}
using namespace GALAXY;

#include "Maths/Maths.h"
#include <memory>
template <typename T>
using Weak = std::weak_ptr<T>;
template <typename T>
using Shared = std::shared_ptr<T>;
template <typename T>
using Unique = std::unique_ptr<T>;

#ifdef GALAXY_EXPORTS
#define GALAXY_API __declspec(dllexport)
#else
#define GALAXY_API __declspec(dllimport)
#endif

#ifdef GALAXY_EXPORTS
	#ifdef ENABLE_REFLECTION
		#include <rttr/registration_friend.h>
		#define REFLECTION_FRIEND RTTR_REGISTRATION_FRIEND
	#else
		#define REFLECTION_FRIEND
	#endif
#endif