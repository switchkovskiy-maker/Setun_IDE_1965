#ifndef TernaryTypesH
#define TernaryTypesH

#include <System.Types.hpp>
#include <vector>
#include <map>

// Предварительное объявление
class TCircuitElement;

// Троичные значения
enum class TTernary { NEG = -1, ZERO = 0, POS = 1 };

// Типы элементов согласно книге
enum class TElementType {
    MAGNETIC_AMPLIFIER,          // Магнитный усилитель
    MAGNETIC_AMPLIFIER_POWER,    // Мощный магнитный усилитель  
    TERNARY_ELEMENT,             // Троичный элемент
    TERNARY_TRIGGER,             // Троичный триггер (стр. 55)
    TERNARY_ADDER,               // Троичный сумматор (стр. 58)
    HALF_ADDER,                  // Полусумматор (стр. 58)
    SHIFT_REGISTER,              // Сдвигающий регистр (стр. 45)
    DECODER,                     // Дешифратор (стр. 57)
    COUNTER,                     // Счетчик (стр. 59)
    DISTRIBUTOR,                 // Распределитель импульсов (стр. 56)
    SWITCH,                      // Переключатель (стр. 55)
    LOGIC_AND,                   // Схема И (стр. 49)
    LOGIC_OR,                    // Схема ИЛИ (стр. 49)
    LOGIC_INHIBIT,               // Схема запрета (стр. 47)
    GENERATOR,                   // Генератор единиц (стр. 50)
    SUBCIRCUIT                   // Подсхема (новый тип для группировки)
};

// Стили линий согласно книге (стр. 49)
enum class TLineStyle {
    POSITIVE_CONTROL,    // Положительная обмотка - стрелка к квадрату
    NEGATIVE_CONTROL,    // Отрицательная обмотка - линия, перечеркивающая квадрат
    OUTPUT_LINE,         // Выходная линия
    INTERNAL_CONNECTION  // Внутреннее соединение
};

// Базовая структура точки соединения
struct TConnectionPoint {
    TCircuitElement* Owner;
    int X, Y;
    TTernary Value;
    bool IsInput;
    TLineStyle LineStyle;
    double RelX, RelY;  // Относительные координаты (0-1)
    
    TConnectionPoint(TCircuitElement* owner, int x, int y, TTernary val, bool isInput, TLineStyle style) 
        : Owner(owner), X(x), Y(y), Value(val), IsInput(isInput), LineStyle(style), RelX(0), RelY(0) {}
        
    // Конструктор по умолчанию для обратной совместимости
    TConnectionPoint(int x, int y, TTernary val, bool isInput, TLineStyle style) 
        : Owner(nullptr), X(x), Y(y), Value(val), IsInput(isInput), LineStyle(style), RelX(0), RelY(0) {}
};

#endif