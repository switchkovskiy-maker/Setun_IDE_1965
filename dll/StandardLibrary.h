#ifndef StandardLibraryH
#define StandardLibraryH

#include "ComponentLibrary.h"

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

extern "C" {
    DLL_EXPORT bool __stdcall RegisterStandardLibrary(TLibraryManager* libraryManager);
    DLL_EXPORT void __stdcall UnregisterStandardLibrary(TLibraryManager* libraryManager);
}

#endif