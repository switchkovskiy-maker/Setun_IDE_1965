#include "StandardLibrary.h"

void RegisterStandardLibrary() {
    auto standardLib = std::make_unique<TComponentLibrary>(
        "Standard", "Стандартная библиотека элементов Сетунь 1965");
    
    // Регистрируем все элементы из оригинальной реализации
    standardLib->RegisterElement("Магнитный усилитель (простой) - стр. 43",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TMagneticAmplifier(Id, X, Y, false);
        });
    
    standardLib->RegisterElement("Магнитный усилитель (мощный) - стр. 49",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TMagneticAmplifier(Id, X, Y, true);
        });
    
    standardLib->RegisterElement("Троичный элемент - стр. 50",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TTernaryElement(Id, X, Y);
        });
    
    standardLib->RegisterElement("Троичный триггер - стр. 55",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TTernaryTrigger(Id, X, Y);
        });
    
    standardLib->RegisterElement("Полусумматор - стр. 58",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new THalfAdder(Id, X, Y);
        });
    
    standardLib->RegisterElement("Троичный сумматор - стр. 58",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TTernaryAdder(Id, X, Y);
        });
    
    standardLib->RegisterElement("Дешифратор кода - стр. 57",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TDecoder(Id, X, Y, 2);
        });
    
    standardLib->RegisterElement("Троичный счетчик - стр. 59",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TCounter(Id, X, Y, 3);
        });
    
    standardLib->RegisterElement("Сдвигающий регистр - стр. 45",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TShiftRegister(Id, X, Y, 4);
        });
    
    standardLib->RegisterElement("Логический элемент И - стр. 49",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TLogicAnd(Id, X, Y);
        });
    
    standardLib->RegisterElement("Логический элемент ИЛИ - стр. 49",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TLogicOr(Id, X, Y);
        });
    
    standardLib->RegisterElement("Схема запрета - стр. 47",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TLogicInhibit(Id, X, Y);
        });
    
    standardLib->RegisterElement("Генератор единиц - стр. 50",
        [](int Id, int X, int Y) -> TCircuitElement* {
            return new TGenerator(Id, X, Y);
        });
    
    LibraryManager->RegisterLibrary(std::move(standardLib));
}