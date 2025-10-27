#include "StandardLibrary.h"
#include "CircuitElements.h"
#include <memory>

// Внешняя функция для регистрации библиотеки в DLL
extern "C" __declspec(dllexport) bool __stdcall RegisterStandardLibrary(TLibraryManager* LibraryManager) {
    if (!LibraryManager) return false;

    try {
        auto standardLib = std::make_unique<TComponentLibrary>(
            "Standard", "Стандартная библиотека элементов Сетунь 1965", "1.0");

        // Регистрируем все элементы с правильными параметрами
        standardLib->RegisterElement<TMagneticAmplifier>(
            "Магнитный усилитель (простой)", 
            "Простой магнитный усилитель", 
            "Усилители", 80, 60);

        standardLib->RegisterElement<TMagneticAmplifier>(
            "Магнитный усилитель (мощный)", 
            "Мощный магнитный усилитель", 
            "Усилители", 80, 60);

        standardLib->RegisterElement<TTernaryElement>(
            "Троичный элемент", 
            "Базовый троичный элемент", 
            "Логика", 80, 60);

        standardLib->RegisterElement<TTernaryTrigger>(
            "Троичный триггер", 
            "Троичный триггер хранения состояния", 
            "Память", 120, 80);

        standardLib->RegisterElement<THalfAdder>(
            "Полусумматор", 
            "Троичный полусумматор", 
            "Арифметика", 80, 60);

        standardLib->RegisterElement<TTernaryAdder>(
            "Троичный сумматор", 
            "Троичный сумматор с переносом", 
            "Арифметика", 100, 80);

        standardLib->RegisterElement<TDecoder>(
            "Дешифратор", 
            "Дешифратор троичного кода", 
            "Логика", 80, 100);

        standardLib->RegisterElement<TCounter>(
            "Счетчик", 
            "Троичный счетчик", 
            "Память", 140, 60);

        standardLib->RegisterElement<TShiftRegister>(
            "Сдвигающий регистр", 
            "4-битный сдвигающий регистр", 
            "Память", 100, 60);

        standardLib->RegisterElement<TLogicAnd>(
            "Логическое И", 
            "Логический элемент И", 
            "Логика", 60, 40);

        standardLib->RegisterElement<TLogicOr>(
            "Логическое ИЛИ", 
            "Логический элемент ИЛИ", 
            "Логика", 60, 40);

        standardLib->RegisterElement<TLogicInhibit>(
            "Запрет", 
            "Схема запрета", 
            "Логика", 80, 60);

        standardLib->RegisterElement<TGenerator>(
            "Генератор", 
            "Генератор единиц", 
            "Источники", 50, 30);

        LibraryManager->RegisterLibrary(std::move(standardLib));
        return true;
    }
    catch (...) {
        return false;
    }
}

// Функция для отмены регистрации (опционально)
extern "C" __declspec(dllexport) void __stdcall UnregisterStandardLibrary(TLibraryManager* LibraryManager) {
    if (LibraryManager) {
        try {
            LibraryManager->UnregisterLibrary("Standard");
        }
        catch (...) {
            // Игнорируем ошибки при выгрузке
        }
    }
}

// Точка входа DLL (опционально)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}