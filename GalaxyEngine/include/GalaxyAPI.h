#pragma once
namespace GALAXY {}
using namespace GALAXY;

#ifdef GALAXY_EXPORTS
#define GALAXY_API __declspec(dllexport)
#else
#define GALAXY_API __declspec(dllimport)
#endif

#define ENABLE_MULTITHREAD