#pragma once

#ifdef _WIN32
#define EXPORT_FUNC extern "C" __declspec(dllexport) inline
#elif defined(__linux__)
#define EXPORT_FUNC extern "C" __attribute__((visibility("default")))
#endif

#define BODY_MACRO_COMBINE_INNER(A,B,C,D) A##B##C##D
#define BODY_MACRO_COMBINE(A,B,C,D) BODY_MACRO_COMBINE_INNER(A,B,C,D)

#define GENERATED_BODY(...) BODY_MACRO_COMBINE(CURRENT_FILE_ID,_,__LINE__,_GENERATED_BODY)
#define GPROPERTY(...)
#define GFUNCTION(...)
#define GCLASS()
#define END_FILE()