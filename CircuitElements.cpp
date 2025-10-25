﻿#include "CircuitElements.h"
#include <Vcl.Graphics.hpp>
#include <math.h>

// Троичный триггер (стр. 55-56, рис. 10)
TTernaryTrigger::TTernaryTrigger(int AId, int X, int Y)
    : TCircuitElement(AId, "Trig", TElementType::TERNARY_TRIGGER, X, Y),
      FStoredState(TTernary::ZERO) {

    FBounds = TRect(X, Y, X + 120, Y + 80);

    // Входы: установка в 1, установка в -1, сброс
    FInputs.push_back(TConnectionPoint(X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(X-15, Y+60, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));

    // Выходы: прямой и инверсный
    FOutputs.push_back(TConnectionPoint(X+135, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    FOutputs.push_back(TConnectionPoint(X+135, Y+50, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TTernaryTrigger::Calculate() {
    // Логика триггера: приоритет сброса, затем установка
    if (FInputs.size() >= 3) {
        TTernary setPos = FInputs[0].Value;
        TTernary setNeg = FInputs[1].Value;
        TTernary reset = FInputs[2].Value;

        if (reset == TTernary::POS) {
            FStoredState = TTernary::ZERO;
        } else if (setPos == TTernary::POS) {
            FStoredState = TTernary::POS;
        } else if (setNeg == TTernary::POS) {
            FStoredState = TTernary::NEG;
        }
        // Иначе сохраняем состояние

        FOutputs[0].Value = FStoredState;
        // Инверсный выход
        FOutputs[1].Value = (FStoredState == TTernary::POS) ? TTernary::NEG :
                           (FStoredState == TTernary::NEG) ? TTernary::POS : TTernary::ZERO;
    }
}

void TTernaryTrigger::Draw(TCanvas* Canvas) {
    // Рисуем два троичных элемента с обратной связью согласно рис. 10
    int centerX = (FBounds.Left + FBounds.Right) / 2;
    int centerY = (FBounds.Top + FBounds.Bottom) / 2;

    // Первый элемент (верхний)
    TRect elem1 = TRect(centerX - 40, centerY - 35, centerX - 5, centerY - 5);
    // Второй элемент (нижний, мощный)
    TRect elem2 = TRect(centerX + 5, centerY + 5, centerX + 40, centerY + 35);

    // Рисуем элементы
    Canvas->Rectangle(elem1.Left, elem1.Top, elem1.Right, elem1.Bottom);
    // Мощный элемент - двойной контур
    Canvas->Pen->Width = 2;
    Canvas->Rectangle(elem2.Left, elem2.Top, elem2.Right, elem2.Bottom);
    Canvas->Pen->Width = 1;
    Canvas->Rectangle(elem2.Left+2, elem2.Top+2, elem2.Right-2, elem2.Bottom-2);

    // Обратные связи
    Canvas->Pen->Color = clBlue;
    Canvas->MoveTo(elem1.Right, elem1.Top + 10);
    Canvas->LineTo(elem2.Left, elem2.Bottom - 10);
    Canvas->MoveTo(elem2.Left, elem2.Top + 10);
    Canvas->LineTo(elem1.Right, elem1.Bottom - 10);
    Canvas->Pen->Color = clBlack;

    // Входы и выходы
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Подписи
    Canvas->Font->Size = 7;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Trig");
    Canvas->TextOut(FInputs[0].X - 20, FInputs[0].Y - 5, "Set+");
    Canvas->TextOut(FInputs[1].X - 20, FInputs[1].Y - 5, "Set-");
    Canvas->TextOut(FInputs[2].X - 20, FInputs[2].Y - 5, "Rst");
    Canvas->TextOut(FOutputs[0].X + 5, FOutputs[0].Y - 5, "Q");
    Canvas->TextOut(FOutputs[1].X + 5, FOutputs[1].Y - 5, "Q~");

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}

void TTernaryTrigger::SetState(TTernary State) {
    FStoredState = State;
}

void TTernaryTrigger::Reset() {
    FStoredState = TTernary::ZERO;
}

// Полусумматор (стр. 58, рис. 13)
THalfAdder::THalfAdder(int AId, int X, int Y)
    : TCircuitElement(AId, "HAdd", TElementType::HALF_ADDER, X, Y) {

    FBounds = TRect(X, Y, X + 80, Y + 60);

    FInputs.push_back(TConnectionPoint(X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(X+95, Y+20, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    FOutputs.push_back(TConnectionPoint(X+95, Y+40, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void THalfAdder::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 2) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;

        // Таблица сложения из книги (стр. 58)
        if (a == TTernary::ZERO && b == TTernary::ZERO) {
            FOutputs[0].Value = TTernary::ZERO; // Сумма
            FOutputs[1].Value = TTernary::ZERO; // Перенос
        } else if (a == TTernary::ZERO && b == TTernary::POS) {
            FOutputs[0].Value = TTernary::POS;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (a == TTernary::ZERO && b == TTernary::NEG) {
            FOutputs[0].Value = TTernary::NEG;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (a == TTernary::POS && b == TTernary::ZERO) {
            FOutputs[0].Value = TTernary::POS;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (a == TTernary::POS && b == TTernary::POS) {
            FOutputs[0].Value = TTernary::ZERO;
            FOutputs[1].Value = TTernary::POS;
        } else if (a == TTernary::POS && b == TTernary::NEG) {
            FOutputs[0].Value = TTernary::ZERO;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (a == TTernary::NEG && b == TTernary::ZERO) {
            FOutputs[0].Value = TTernary::NEG;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (a == TTernary::NEG && b == TTernary::POS) {
            FOutputs[0].Value = TTernary::ZERO;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (a == TTernary::NEG && b == TTernary::NEG) {
            FOutputs[0].Value = TTernary::ZERO;
            FOutputs[1].Value = TTernary::NEG;
        }
    }
}

void THalfAdder::Draw(TCanvas* Canvas) {
    // Рисуем согласно книжному обозначению полусумматора
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    // Внутренние обозначения
    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 10, FBounds.Top + 10, "Σ");
    Canvas->TextOut(FBounds.Left + 10, FBounds.Top + 30, "C");

    // Входы и выходы
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Подписи
    Canvas->TextOut(FInputs[0].X - 10, FInputs[0].Y - 5, "A");
    Canvas->TextOut(FInputs[1].X - 10, FInputs[1].Y - 5, "B");
    Canvas->TextOut(FOutputs[0].X + 5, FOutputs[0].Y - 5, "S");
    Canvas->TextOut(FOutputs[1].X + 5, FOutputs[1].Y - 5, "C");

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}

// Троичный сумматор (стр. 58-59, рис. 14)
TTernaryAdder::TTernaryAdder(int AId, int X, int Y)
    : TCircuitElement(AId, "TAdd", TElementType::TERNARY_ADDER, X, Y) {

    FBounds = TRect(X, Y, X + 100, Y + 80);

    FInputs.push_back(TConnectionPoint(X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(X-15, Y+60, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(X+115, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    FOutputs.push_back(TConnectionPoint(X+115, Y+50, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TTernaryAdder::Calculate() {
    if (FInputs.size() >= 3 && FOutputs.size() >= 2) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;
        TTernary carryIn = FInputs[2].Value;

        // Преобразуем в числовые значения для сложения
        int a_val = (a == TTernary::POS) ? 1 : (a == TTernary::NEG) ? -1 : 0;
        int b_val = (b == TTernary::POS) ? 1 : (b == TTernary::NEG) ? -1 : 0;
        int c_val = (carryIn == TTernary::POS) ? 1 : (carryIn == TTernary::NEG) ? -1 : 0;

        int sum = a_val + b_val + c_val;

        // Преобразуем обратно в троичную систему с переносом
        if (sum >= 2) {
            FOutputs[0].Value = TTernary::POS; // Сумма
            FOutputs[1].Value = TTernary::POS; // Перенос
        } else if (sum == 1) {
            FOutputs[0].Value = TTernary::POS;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (sum == 0) {
            FOutputs[0].Value = TTernary::ZERO;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (sum == -1) {
            FOutputs[0].Value = TTernary::NEG;
            FOutputs[1].Value = TTernary::ZERO;
        } else if (sum <= -2) {
            FOutputs[0].Value = TTernary::NEG;
            FOutputs[1].Value = TTernary::NEG;
        }
    }
}

void TTernaryAdder::Draw(TCanvas* Canvas) {
    // Рисуем два полусумматора согласно рис. 14
    int centerX = (FBounds.Left + FBounds.Right) / 2;

    // Первый полусумматор
    TRect ha1 = TRect(FBounds.Left + 10, FBounds.Top + 10, centerX - 5, FBounds.Top + 40);
    // Второй полусумматор
    TRect ha2 = TRect(centerX + 5, FBounds.Top + 30, FBounds.Right - 10, FBounds.Top + 60);

    Canvas->Rectangle(ha1.Left, ha1.Top, ha1.Right, ha1.Bottom);
    Canvas->Rectangle(ha2.Left, ha2.Top, ha2.Right, ha2.Bottom);

    // Соединения между полусумматорами
    Canvas->Pen->Color = clBlue;
    Canvas->MoveTo(ha1.Right, ha1.Top + 15);
    Canvas->LineTo(ha2.Left, ha2.Top + 15);
    Canvas->MoveTo(ha1.Right, ha1.Top + 25);
    Canvas->LineTo(ha2.Left, ha2.Bottom - 10);
    Canvas->Pen->Color = clBlack;

    // Входы и выходы
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Подписи
    Canvas->Font->Size = 7;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "TAdd");
    Canvas->TextOut(FInputs[0].X - 10, FInputs[0].Y - 5, "A");
    Canvas->TextOut(FInputs[1].X - 10, FInputs[1].Y - 5, "B");
    Canvas->TextOut(FInputs[2].X - 10, FInputs[2].Y - 5, "Cin");
    Canvas->TextOut(FOutputs[0].X + 5, FOutputs[0].Y - 5, "S");
    Canvas->TextOut(FOutputs[1].X + 5, FOutputs[1].Y - 5, "Cout");

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}

// Дешифратор кода (стр. 57, рис. 12)
TDecoder::TDecoder(int AId, int X, int Y, int InputBits)
    : TCircuitElement(AId, "Dec", TElementType::DECODER, X, Y),
      FInputBits(InputBits), FOutputCount((int)pow(3, InputBits)) {

    FBounds = TRect(X, Y, X + 80, Y + 30 + FOutputCount * 15);

    // Входы
    for (int i = 0; i < FInputBits; i++) {
        FInputs.push_back(TConnectionPoint(X-15, Y+20 + i*15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    }

    // Выходы
    for (int i = 0; i < FOutputCount; i++) {
        FOutputs.push_back(TConnectionPoint(X+95, Y+15 + i*15, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    }
}

void TDecoder::Calculate() {
    // Определяем активный выход на основе входных значений
    int activeOutput = 0;
    for (int i = 0; i < FInputBits; i++) {
        if (i < FInputs.size()) {
            int digitValue = (FInputs[i].Value == TTernary::POS) ? 2 :
                           (FInputs[i].Value == TTernary::ZERO) ? 1 : 0;
            activeOutput = activeOutput * 3 + digitValue;
        }
    }

    // Активируем только один выход
    for (int i = 0; i < FOutputs.size(); i++) {
        FOutputs[i].Value = (i == activeOutput) ? TTernary::POS : TTernary::ZERO;
    }
}

void TDecoder::Draw(TCanvas* Canvas) {
    // Рисуем прямоугольник дешифратора
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    // Входы
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }

    // Выходы
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Подписи
    Canvas->Font->Size = 7;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Dec");

    // Подписи выходов (троичные коды)
    for (int i = 0; i < FOutputs.size(); i++) {
        String code;
        int temp = i;
        for (int j = 0; j < FInputBits; j++) {
            int digit = temp % 3;
            code = String((digit == 2) ? "1" : (digit == 1) ? "0" : "I") + code;
            temp /= 3;
        }
        Canvas->TextOut(FOutputs[i].X + 5, FOutputs[i].Y - 5, code);
    }

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}

// Троичный счетчик (стр. 59, рис. 15)
TCounter::TCounter(int AId, int X, int Y, int BitCount)
    : TCircuitElement(AId, "Cnt", TElementType::COUNTER, X, Y),
      FCount(0), FMaxCount((int)pow(3, BitCount) - 1) {

    FBounds = TRect(X, Y, X + 40 + BitCount * 25, Y + 60);

    FInputs.push_back(TConnectionPoint(X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL)); // +1
    FInputs.push_back(TConnectionPoint(X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL)); // -1
    FOutputs.push_back(TConnectionPoint(X+FBounds.Width()+5, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TCounter::Calculate() {
    if (FInputs.size() >= 2) {
        TTernary inc = FInputs[0].Value;
        TTernary dec = FInputs[1].Value;

        if (inc == TTernary::POS) {
            FCount = (FCount < FMaxCount) ? FCount + 1 : 0;
        } else if (dec == TTernary::POS) {
            FCount = (FCount > 0) ? FCount - 1 : FMaxCount;
        }

        // Устанавливаем выход на основе текущего счета
        FOutputs[0].Value = (FCount > FMaxCount/2) ? TTernary::POS :
                           (FCount < FMaxCount/2) ? TTernary::NEG : TTernary::ZERO;
    }
}

void TCounter::Draw(TCanvas* Canvas) {
    // Рисуем последовательность триггеров (битов счетчика)
    int bitWidth = 25;
    for (int i = 0; i < (FMaxCount + 1); i++) {
        int left = FBounds.Left + i * bitWidth;
        TRect bitRect = TRect(left, FBounds.Top + 10, left + bitWidth - 2, FBounds.Bottom - 10);

        // Подсвечиваем активный бит
        if (i == FCount) {
            Canvas->Brush->Color = clYellow;
            Canvas->Rectangle(bitRect.Left, bitRect.Top, bitRect.Right, bitRect.Bottom);
            Canvas->Brush->Color = clWhite;
        } else {
            Canvas->Rectangle(bitRect.Left, bitRect.Top, bitRect.Right, bitRect.Bottom);
        }

        // Номер бита
        Canvas->Font->Size = 6;
        Canvas->TextOut(bitRect.Left + 2, bitRect.Top + 2, IntToStr(i));
    }

    // Входы и выходы
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Подписи
    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Cnt");
    Canvas->TextOut(FInputs[0].X - 10, FInputs[0].Y - 5, "+1");
    Canvas->TextOut(FInputs[1].X - 10, FInputs[1].Y - 5, "-1");

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}

void TCounter::Reset() {
    FCount = 0;
}

// Логический элемент И (стр. 49-50, рис. 8)
TLogicAnd::TLogicAnd(int AId, int X, int Y)
    : TCircuitElement(AId, "AND", TElementType::LOGIC_AND, X, Y) {

    FBounds = TRect(X, Y, X + 60, Y + 40);
    FInputs.push_back(TConnectionPoint(X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(X-15, Y+25, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(X+75, Y+20, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TLogicAnd::Calculate() {
    if (FInputs.size() >= 2) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;

        // Логика И: выход POS только если оба входа POS
        FOutputs[0].Value = (a == TTernary::POS && b == TTernary::POS) ? TTernary::POS : TTernary::ZERO;
    }
}

void TLogicAnd::Draw(TCanvas* Canvas) {
    // Рисуем элемент И с постоянно возбужденной обмоткой запрета
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    // Рисуем символ И
    Canvas->Font->Size = 10;
    Canvas->TextOut(FBounds.Left + 20, FBounds.Top + 10, "&");

    // Входы и выходы
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}

// Логический элемент ИЛИ (стр. 49, рис. 7)
TLogicOr::TLogicOr(int AId, int X, int Y)
    : TCircuitElement(AId, "OR", TElementType::LOGIC_OR, X, Y) {

    FBounds = TRect(X, Y, X + 60, Y + 40);
    FInputs.push_back(TConnectionPoint(X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(X-15, Y+25, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(X+75, Y+20, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TLogicOr::Calculate() {
    if (FInputs.size() >= 2) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;

        // Логика ИЛИ: выход POS если хотя бы один вход POS
        FOutputs[0].Value = (a == TTernary::POS || b == TTernary::POS) ? TTernary::POS : TTernary::ZERO;
    }
}

void TLogicOr::Draw(TCanvas* Canvas) {
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    // Рисуем символ ИЛИ
    Canvas->Font->Size = 10;
    Canvas->TextOut(FBounds.Left + 20, FBounds.Top + 10, "≥1");

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}

// Схема запрета (стр. 47-48, рис. 6)
TLogicInhibit::TLogicInhibit(int AId, int X, int Y)
    : TCircuitElement(AId, "INH", TElementType::LOGIC_INHIBIT, X, Y) {

    FBounds = TRect(X, Y, X + 80, Y + 60);
    FInputs.push_back(TConnectionPoint(X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL)); // A
    FInputs.push_back(TConnectionPoint(X-15, Y+40, TTernary::ZERO, true, TLineStyle::NEGATIVE_CONTROL)); // B (запрет)
    FOutputs.push_back(TConnectionPoint(X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TLogicInhibit::Calculate() {
    if (FInputs.size() >= 2) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;

        // Запрет: выход A только если B не активен
        FOutputs[0].Value = (b == TTernary::ZERO) ? a : TTernary::ZERO;
    }
}

void TLogicInhibit::Draw(TCanvas* Canvas) {
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    // Специальное обозначение запрета
    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 10, FBounds.Top + 10, "INH");
    Canvas->TextOut(FBounds.Left + 10, FBounds.Top + 25, "A");
    Canvas->TextOut(FBounds.Left + 10, FBounds.Top + 40, "B");

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}

// Генератор единиц (стр. 50, рис. 8)
TGenerator::TGenerator(int AId, int X, int Y)
    : TCircuitElement(AId, "Gen", TElementType::GENERATOR, X, Y) {

    FBounds = TRect(X, Y, X + 50, Y + 30);
    FOutputs.push_back(TConnectionPoint(X+65, Y+15, TTernary::POS, false, TLineStyle::OUTPUT_LINE));
}

void TGenerator::Calculate() {
    // Генератор всегда выдает POS
    FOutputs[0].Value = TTernary::POS;
}

void TGenerator::Draw(TCanvas* Canvas) {
    // Рисуем генератор с положительной обмоткой
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Gen");
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 20, "1");

    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    // Рисуем точки соединения
    DrawConnectionPoints(Canvas);
}
