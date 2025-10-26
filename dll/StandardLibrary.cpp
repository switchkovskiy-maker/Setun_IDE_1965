#define BUILD_DLL
#include "StandardLibrary.h"
#include "CircuitElement.h"
#include "CircuitElements.h"
#include <memory>

// Глобальный указатель на библиотеку для возможности выгрузки
static TComponentLibrary* GStandardLibrary = nullptr;

extern "C" {

DLL_EXPORT bool __stdcall RegisterStandardLibrary(TLibraryManager* libraryManager) {
    if (!libraryManager) {
        return false;
    }

    try {
        auto standardLib = std::make_unique<TComponentLibrary>(
            "Standard", "Стандартная библиотека элементов Сетунь 1965", "1.0");

        // Регистрируем все элементы через шаблонный метод
        standardLib->RegisterElement<TTernaryTrigger>(
            "Троичный триггер", "Троичный триггер - стр. 55", "Память", 120, 80);
        
        standardLib->RegisterElement<THalfAdder>(
            "Полусумматор", "Полусумматор - стр. 58", "Арифметика", 80, 60);
            
        standardLib->RegisterElement<TTernaryAdder>(
            "Троичный сумматор", "Троичный сумматор - стр. 58", "Арифметика", 100, 80);
            
        standardLib->RegisterElement<TDecoder>(
            "Дешифратор кода", "Дешифратор кода - стр. 57", "Логика");
            
        standardLib->RegisterElement<TCounter>(
            "Троичный счетчик", "Троичный счетчик - стр. 59", "Память");
            
        standardLib->RegisterElement<TLogicAnd>(
            "Логический элемент И", "Логический элемент И - стр. 49", "Логика");
            
        standardLib->RegisterElement<TLogicOr>(
            "Логический элемент ИЛИ", "Логический элемент ИЛИ - стр. 49", "Логика");
            
        standardLib->RegisterElement<TLogicInhibit>(
            "Схема запрета", "Схема запрета - стр. 47", "Логика", 80, 60);
            
        standardLib->RegisterElement<TGenerator>(
            "Генератор единиц", "Генератор единиц - стр. 50", "Источники", 50, 30);

        // Сохраняем указатель для возможности выгрузки
        GStandardLibrary = standardLib.get();
        
        libraryManager->RegisterLibrary(std::move(standardLib));
        return true;
    }
    catch (...) {
        return false;
    }
}

DLL_EXPORT void __stdcall UnregisterStandardLibrary(TLibraryManager* libraryManager) {
    if (libraryManager && GStandardLibrary) {
        libraryManager->UnregisterLibrary(GStandardLibrary->Name);
        GStandardLibrary = nullptr;
    }
}

} // extern "C"