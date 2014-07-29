//#pragma once

//#define CALIPSO_DLL_EXPORTS

#ifdef CALIPSO_DLL_EXPORTS
#define  CALIPSO_DLL_API __declspec(dllexport)
#else
#define  CALIPSO_DLL_API __declspec(dllimport)
#endif

CALIPSO_DLL_API int calipso_main();
CALIPSO_DLL_API void calipso_start_thread();