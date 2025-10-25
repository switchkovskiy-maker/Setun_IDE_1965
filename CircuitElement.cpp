#include "CircuitElement.h"
#include <Vcl.Graphics.hpp>
#include <math.h>

// Базовый элемент
TCircuitElement::TCircuitElement(int AId, const String& AName, TElementType AType, int X, int Y)
    : FId(AId), FName(AName), FType(AType), FCurrentState(TTernary::ZERO) {
    FBounds = TRect(X, Y, X + 80, Y + 60);
}

void TCircuitElement::Draw(TCanvas* Canvas) {
    switch (FType) {
        case TElementType::MAGNETIC_AMPLIFIER:
            DrawMagneticAmplifier(Canvas, false);
            break;
        case TElementType::MAGNETIC_AMPLIFIER_POWER:
            DrawMagneticAmplifier(Canvas, true);
            break;
        case TElementType::TERNARY_ELEMENT:
            DrawTernaryElement(Canvas);
            break;
        default:
            // Рисуем базовый прямоугольник для неизвестных элементов
            Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);
            Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, FName);
    }

    // Рисуем точки соединения (входы и выходы) для всех элементов
    DrawConnectionPoints(Canvas);
}

void TCircuitElement::DrawConnectionPoints(TCanvas* Canvas) {
    // Рисуем входы (зеленые кружки)
    Canvas->Brush->Color = clGreen;
    for (const auto& input : FInputs) {
        Canvas->Ellipse(input.X - 4, input.Y - 4, input.X + 4, input.Y + 4);
    }

    // Рисуем выходы (синие квадраты)
    Canvas->Brush->Color = clBlue;
    for (const auto& output : FOutputs) {
        Canvas->Rectangle(output.X - 4, output.Y - 4, output.X + 4, output.Y + 4);
    }

    Canvas->Brush->Color = clWhite;
}

TConnectionPoint* TCircuitElement::GetConnectionAt(int X, int Y) {
    // Проверяем входы
    for (auto& input : FInputs) {
        if (abs(input.X - X) < 8 && abs(input.Y - Y) < 8) {
            return &input;
        }
    }

    // Проверяем выходы
    for (auto& output : FOutputs) {
        if (abs(output.X - X) < 8 && abs(output.Y - Y) < 8) {
            return &output;
        }
    }

    return nullptr;
}

// Рисуем магнитный усилитель согласно книге (стр. 49)
void TCircuitElement::DrawMagneticAmplifier(TCanvas* Canvas, bool IsPowerful) {
    // Основной квадрат
    if (IsPowerful) {
        // Мощный усилитель - двойной контур (стр. 49)
        Canvas->Pen->Width = 2;
        Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);
        Canvas->Pen->Width = 1;
        Canvas->Rectangle(FBounds.Left+2, FBounds.Top+2, FBounds.Right-2, FBounds.Bottom-2);
    } else {
        // Простой усилитель - одинарный контур
        Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);
    }

    // Рисуем управляющие обмотки согласно книжным обозначениям
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }

    // Рисуем выходные линии
    for (const auto& output : FOutputs) {
        Canvas->Pen->Color = clBlack;
        Canvas->MoveTo(FBounds.Right, output.Y);
        Canvas->LineTo(output.X, output.Y);
    }

    // Подпись элемента
    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, IsPowerful ? "MA+" : "MA");
}

// Рисуем троичный элемент согласно рис. 9 на стр. 50
void TCircuitElement::DrawTernaryElement(TCanvas* Canvas) {
    // Троичный элемент состоит из двух усилителей с взаимным запретом
    int centerX = (FBounds.Left + FBounds.Right) / 2;
    int centerY = (FBounds.Top + FBounds.Bottom) / 2;

    // Рисуем два квадрата (усилителя)
    TRect rect1 = TRect(centerX - 30, centerY - 25, centerX - 5, centerY);
    TRect rect2 = TRect(centerX + 5, centerY, centerX + 30, centerY + 25);

    Canvas->Rectangle(rect1.Left, rect1.Top, rect1.Right, rect1.Bottom);
    Canvas->Rectangle(rect2.Left, rect2.Top, rect2.Right, rect2.Bottom);

    // Взаимные запрещающие связи (перечеркивающие линии)
    DrawCrossingLine(Canvas, rect1.Left, rect1.Bottom, rect2.Right, rect2.Top);
    DrawCrossingLine(Canvas, rect1.Right, rect1.Top, rect2.Left, rect2.Bottom);

    // Входы и выходы
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        Canvas->MoveTo(FBounds.Right, output.Y);
        Canvas->LineTo(output.X, output.Y);
    }

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "TE");
}

// Рисуем линии управления согласно книжным стандартам (стр. 49)
void TCircuitElement::DrawControlLine(TCanvas* Canvas, const TConnectionPoint& Point) {
    Canvas->Pen->Color = clBlack;

    switch (Point.LineStyle) {
        case TLineStyle::POSITIVE_CONTROL:
            // Положительная обмотка - стрелка к квадрату
            Canvas->MoveTo(Point.X, Point.Y);
            Canvas->LineTo(FBounds.Left, Point.Y);
            // Рисуем стрелку
            Canvas->MoveTo(FBounds.Left, Point.Y);
            Canvas->LineTo(FBounds.Left - 5, Point.Y - 3);
            Canvas->MoveTo(FBounds.Left, Point.Y);
            Canvas->LineTo(FBounds.Left - 5, Point.Y + 3);
            break;

        case TLineStyle::NEGATIVE_CONTROL:
            // Отрицательная обмотка - линия, перечеркивающая квадрат
            Canvas->MoveTo(Point.X, Point.Y);
            Canvas->LineTo(FBounds.Right + 10, Point.Y);
            // Рисуем кружок на конце
            Canvas->Ellipse(FBounds.Right + 8, Point.Y - 2,
                           FBounds.Right + 12, Point.Y + 2);
            break;

        case TLineStyle::OUTPUT_LINE:
            // Выходная линия
            Canvas->MoveTo(FBounds.Right, Point.Y);
            Canvas->LineTo(Point.X, Point.Y);
            break;

        case TLineStyle::INTERNAL_CONNECTION:
            // Внутреннее соединение - пунктирная линия
            Canvas->Pen->Style = psDash;
            Canvas->MoveTo(FBounds.Left, Point.Y);
            Canvas->LineTo(Point.X, Point.Y);
            Canvas->Pen->Style = psSolid;
            break;
    }
}

// Рисуем перечеркивающую линию (для взаимных запретов)
void TCircuitElement::DrawCrossingLine(TCanvas* Canvas, int X1, int Y1, int X2, int Y2) {
    Canvas->Pen->Color = clRed;
    Canvas->Pen->Style = psDash;
    Canvas->MoveTo(X1, Y1);
    Canvas->LineTo(X2, Y2);
    Canvas->Pen->Style = psSolid;
}

// Магнитный усилитель
TMagneticAmplifier::TMagneticAmplifier(int AId, int X, int Y, bool IsPowerful)
    : TCircuitElement(AId, "MA",
          IsPowerful ? TElementType::MAGNETIC_AMPLIFIER_POWER : TElementType::MAGNETIC_AMPLIFIER,
          X, Y), FIsPowered(false) {

    // Питание (положительная обмотка)
    FInputs.push_back(TConnectionPoint(X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    // Управление (может быть положительной или отрицательной обмоткой)
    FInputs.push_back(TConnectionPoint(X-15, Y+30, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    // Выход
    FOutputs.push_back(TConnectionPoint(X+95, Y+22, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TMagneticAmplifier::Calculate() {
    if (FInputs.size() >= 2) {
        TTernary power = FInputs[0].Value;
        TTernary control = FInputs[1].Value;

        // Упрощенная логика магнитного усилителя
        if (power == TTernary::POS) {
            if (control == TTernary::POS) {
                FOutputs[0].Value = TTernary::POS;
            } else if (control == TTernary::NEG) {
                FOutputs[0].Value = TTernary::NEG;
            } else {
                FOutputs[0].Value = TTernary::ZERO;
            }
        } else {
            FOutputs[0].Value = TTernary::ZERO;
        }
    }
}

// Троичный элемент
TTernaryElement::TTernaryElement(int AId, int X, int Y)
    : TCircuitElement(AId, "TE", TElementType::TERNARY_ELEMENT, X, Y) {

    // Два входа (положительные обмотки)
    FInputs.push_back(TConnectionPoint(X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(X-15, Y+45, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    // Один выход
    FOutputs.push_back(TConnectionPoint(X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TTernaryElement::Calculate() {
    if (FInputs.size() >= 2) {
        TTernary in1 = FInputs[0].Value;
        TTernary in2 = FInputs[1].Value;

        // Логика взаимного запрета
        if (in1 == TTernary::POS && in2 != TTernary::POS) {
            FOutputs[0].Value = TTernary::POS;
        } else if (in1 == TTernary::NEG && in2 != TTernary::NEG) {
            FOutputs[0].Value = TTernary::NEG;
        } else {
            FOutputs[0].Value = TTernary::ZERO;
        }
    }
}

// Сдвигающий регистр с двухфазным питанием (рис. 5, стр. 45)
TShiftRegister::TShiftRegister(int AId, int X, int Y, int BitCount)
    : TCircuitElement(AId, "SR", TElementType::SHIFT_REGISTER, X, Y), FBitCount(BitCount) {

    // Вход данных
    FInputs.push_back(TConnectionPoint(X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    // Тактовый вход
    FInputs.push_back(TConnectionPoint(X-15, Y+45, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    // Выход данных
    FOutputs.push_back(TConnectionPoint(X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
}

void TShiftRegister::Calculate() {
    // Упрощенная логика сдвигающего регистра
    if (FInputs.size() >= 2 && FInputs[1].Value == TTernary::POS) {
        // По фронту тактового импульса сдвигаем данные
        FOutputs[0].Value = FInputs[0].Value;
    }
}

void TShiftRegister::Draw(TCanvas* Canvas) {
    // Рисуем последовательность квадратов (биты регистра)
    int bitWidth = 20;
    for (int i = 0; i < FBitCount; i++) {
        int left = FBounds.Left + i * bitWidth;
        TRect bitRect = TRect(left, FBounds.Top, left + bitWidth - 2, FBounds.Bottom);
        Canvas->Rectangle(bitRect.Left, bitRect.Top, bitRect.Right, bitRect.Bottom);

        // Подписываем биты
        if (i == 0) Canvas->TextOut(bitRect.Left+2, bitRect.Top+2, "IN");
        if (i == FBitCount-1) Canvas->TextOut(bitRect.Left+2, bitRect.Top+2, "OUT");
    }

    // Рисуем соединения между битами
    Canvas->Pen->Color = clBlue;
    for (int i = 0; i < FBitCount - 1; i++) {
        int x1 = FBounds.Left + i * bitWidth + bitWidth - 2;
        int x2 = x1 + 2;
        int y = (FBounds.Top + FBounds.Bottom) / 2;
        Canvas->MoveTo(x1, y);
        Canvas->LineTo(x2, y);
    }
    Canvas->Pen->Color = clBlack;

    // Рисуем входы и выходы
    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "SR");
}
