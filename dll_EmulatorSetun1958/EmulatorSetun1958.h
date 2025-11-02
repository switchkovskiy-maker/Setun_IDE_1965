#ifndef EmulatorSetun1958H
#define EmulatorSetun1958H

#include "ComponentLibrary.h"

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

// Версия библиотеки
#define EMULATOR_SETUN_1958_VERSION "1.0.0"
#define EMULATOR_SETUN_1958_NAME "Emulator-Setun-1958"
#define EMULATOR_SETUN_1958_DESCRIPTION "Эмулятор элементов Сетунь-1958"

extern "C" {
    // Обязательные функции
    DLL_EXPORT bool __stdcall RegisterLibrary(TLibraryManager* libraryManager);
    DLL_EXPORT void __stdcall UnregisterLibrary(TLibraryManager* libraryManager);
    
    // Опциональные информационные функции
    DLL_EXPORT const char* __stdcall GetLibraryName();
    DLL_EXPORT const char* __stdcall GetLibraryVersion();
    DLL_EXPORT const char* __stdcall GetLibraryDescription();
}

// Глобальный указатель для управления библиотекой
extern TComponentLibrary* GEmulatorSetun1958Library;

#endif