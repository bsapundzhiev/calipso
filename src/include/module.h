#ifndef _MODULE_H
#define _MODULE_H

#ifdef _WIN32
#		include <windows.h>
#ifdef WP8
__inline void * Sys_LoadLibrary(const char *path)
{
    int len = strlen(path);
    wchar_t wpath[64];
    MultiByteToWideChar(CP_ACP, 0, path, len, wpath, len);
    wpath[len]= '\0';
    return (void*)LoadPackagedLibrary(wpath, 0);
}
#else
#		define Sys_LoadLibrary(f) (void*)LoadLibrary(f)
#endif
#		define Sys_UnloadLibrary(h) FreeLibrary((HMODULE)h)
#		define Sys_LoadFunction(h,fn) (void*)GetProcAddress((HMODULE)h, fn)
#		define Sys_LibraryError() GetLastError()==0 ? NULL : "LibraryError"
#else
#		include <dlfcn.h>
#		define Sys_LoadLibrary(f) dlopen(f,RTLD_NOW)
#		define Sys_UnloadLibrary(h) dlclose(h)
#		define Sys_LoadFunction(h,fn) dlsym(h,fn)
#		define Sys_LibraryError() dlerror()
#endif

/* MAX loaded */
#define MAX_LOADED_MODULES 	16

/* entry points */
//!FIXME: name refact.
#define MODULE_INIT_FUNCTION 			"pm_init"
#define MODULE_EXIT_FUNCTION 			"pm_exit"
#define MODULE_CONFIGURE_FUNCTION 		"pm_configure"

#ifdef _WIN32
#define CPMOD_API	__declspec(dllexport)
#else
#define CPMOD_API
#endif

#define CPMOD_INITIALIZER()\
CPMOD_API inline void CalipsoModWin32Initializer(calipso_t *c)\
{\
	if(calipso == NULL){\
		printf("calipso is NULL\n");\
		calipso = c;\
	}\
}


int
calipso_set_module_table(calipso_mod_t * mod_table);

/* lookup module table use in init hooks */
cpo_bool
calipso_modules_is_module(const char * module_name);

#endif

