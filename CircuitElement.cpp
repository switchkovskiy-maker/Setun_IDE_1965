#include "CircuitElement.h"
#include <Vcl.Graphics.hpp>
#include <math.h>

#pragma package(smart_init)

TCircuitElement::TCircuitElement(int AId, const String& AName, int X, int Y)
    : FId(AId), FName(AName), FCurrentState(TTernary::ZERO) {
    FBounds = TRect(X, Y, X + 80, Y + 60);
}

void TCircuitElement::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, FName);

    DrawConnectionPoints(Canvas);
}

void TCircuitElement::DrawConnectionPoints(TCanvas* Canvas) {
    Canvas->Brush->Color = clGreen;
    for (const auto& input : FInputs) {
        Canvas->Ellipse(input.X - 4, input.Y - 4, input.X + 4, input.Y + 4);
    }

    Canvas->Brush->Color = clBlue;
    for (const auto& output : FOutputs) {
        Canvas->Rectangle(output.X - 4, output.Y - 4, output.X + 4, output.Y + 4);
    }

    Canvas->Brush->Color = clWhite;
}

TConnectionPoint* TCircuitElement::GetConnectionAt(int X, int Y) {
    for (auto& input : FInputs) {
        if (abs(input.X - X) < 8 && abs(input.Y - Y) < 8) {
            return &input;
        }
    }

    for (auto& output : FOutputs) {
        if (abs(output.X - X) < 8 && abs(output.Y - Y) < 8) {
            return &output;
        }
    }

    return nullptr;
}

void TCircuitElement::DrawMagneticAmplifier(TCanvas* Canvas, bool IsPowerful) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;

    if (IsPowerful) {
        Canvas->Pen->Width = 2;
        Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);
        Canvas->Pen->Width = 1;
        Canvas->Rectangle(FBounds.Left+2, FBounds.Top+2, FBounds.Right-2, FBounds.Bottom-2);
    } else {
        Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);
    }

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }

    for (const auto& output : FOutputs) {
        Canvas->Pen->Color = clBlack;
        Canvas->MoveTo(FBounds.Right, output.Y);
        Canvas->LineTo(output.X, output.Y);
    }

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, IsPowerful ? "MA+" : "MA");

    DrawConnectionPoints(Canvas);
}

void TCircuitElement::DrawTernaryElement(TCanvas* Canvas) {
    int centerX = (FBounds.Left + FBounds.Right) / 2;
    int centerY = (FBounds.Top + FBounds.Bottom) / 2;

    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;

    TRect rect1 = TRect(centerX - 30, centerY - 25, centerX - 5, centerY);
    TRect rect2 = TRect(centerX + 5, centerY, centerX + 30, centerY + 25);

    Canvas->Rectangle(rect1.Left, rect1.Top, rect1.Right, rect1.Bottom);
    Canvas->Rectangle(rect2.Left, rect2.Top, rect2.Right, rect2.Bottom);

    DrawCrossingLine(Canvas, rect1.Left, rect1.Bottom, rect2.Right, rect2.Top);
    DrawCrossingLine(Canvas, rect1.Right, rect1.Top, rect2.Left, rect2.Bottom);

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        Canvas->MoveTo(FBounds.Right, output.Y);
        Canvas->LineTo(output.X, output.Y);
    }

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "TE");

    DrawConnectionPoints(Canvas);
}

void TCircuitElement::DrawControlLine(TCanvas* Canvas, const TConnectionPoint& Point) {
    Canvas->Pen->Color = clBlack;

    switch (Point.LineStyle) {
        case TLineStyle::POSITIVE_CONTROL:
            Canvas->MoveTo(Point.X, Point.Y);
            Canvas->LineTo(FBounds.Left, Point.Y);
            Canvas->MoveTo(FBounds.Left, Point.Y);
            Canvas->LineTo(FBounds.Left - 5, Point.Y - 3);
            Canvas->MoveTo(FBounds.Left, Point.Y);
            Canvas->LineTo(FBounds.Left - 5, Point.Y + 3);
            break;

        case TLineStyle::NEGATIVE_CONTROL:
            Canvas->MoveTo(Point.X, Point.Y);
            Canvas->LineTo(FBounds.Right + 10, Point.Y);
            Canvas->Ellipse(FBounds.Right + 8, Point.Y - 2,
                           FBounds.Right + 12, Point.Y + 2);
            break;

        case TLineStyle::OUTPUT_LINE:
            Canvas->MoveTo(FBounds.Right, Point.Y);
            Canvas->LineTo(Point.X, Point.Y);
            break;

        case TLineStyle::INTERNAL_CONNECTION:
            Canvas->Pen->Style = psDash;
            Canvas->MoveTo(FBounds.Left, Point.Y);
            Canvas->LineTo(Point.X, Point.Y);
            Canvas->Pen->Style = psSolid;
            break;
    }
}

void TCircuitElement::DrawCrossingLine(TCanvas* Canvas, int X1, int Y1, int X2, int Y2) {
    Canvas->Pen->Color = clRed;
    Canvas->Pen->Style = psDash;
    Canvas->MoveTo(X1, Y1);
    Canvas->LineTo(X2, Y2);
    Canvas->Pen->Style = psSolid;
    Canvas->Pen->Color = clBlack;
}
TMagneticAmplifier::TMagneticAmplifier(int AId, int X, int Y, bool IsPowerful)
    : TCircuitElement(AId, "Магнитный усилитель", X, Y), FIsPowered(false) {

    FInputs.push_back(TConnectionPoint(this, X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+30, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+95, Y+22, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TMagneticAmplifier::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
        TTernary power = FInputs[0].Value;
        TTernary control = FInputs[1].Value;

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

TTernaryElement::TTernaryElement(int AId, int X, int Y)
    : TCircuitElement(AId, "Троичный элемент", X, Y) {

    FInputs.push_back(TConnectionPoint(this, X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+45, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TTernaryElement::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
        TTernary in1 = FInputs[0].Value;
        TTernary in2 = FInputs[1].Value;

        if (in1 == TTernary::POS && in2 != TTernary::POS) {
            FOutputs[0].Value = TTernary::POS;
        } else if (in1 == TTernary::NEG && in2 != TTernary::NEG) {
            FOutputs[0].Value = TTernary::NEG;
        } else {
            FOutputs[0].Value = TTernary::ZERO;
        }
    }
}

TShiftRegister::TShiftRegister(int AId, int X, int Y, int BitCount)
    : TCircuitElement(AId, "Сдвигающий регистр", X, Y), FBitCount(BitCount) {

    FInputs.push_back(TConnectionPoint(this, X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+45, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TShiftRegister::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 1 && FInputs[1].Value == TTernary::POS) {
        FOutputs[0].Value = FInputs[0].Value;
    }
}

void TShiftRegister::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;

    int bitWidth = 20;
    for (int i = 0; i < FBitCount; i++) {
        int left = FBounds.Left + i * bitWidth;
        TRect bitRect = TRect(left, FBounds.Top, left + bitWidth - 2, FBounds.Bottom);
        Canvas->Rectangle(bitRect.Left, bitRect.Top, bitRect.Right, bitRect.Bottom);

        if (i == 0) Canvas->TextOut(bitRect.Left+2, bitRect.Top+2, "IN");
        if (i == FBitCount-1) Canvas->TextOut(bitRect.Left+2, bitRect.Top+2, "OUT");
    }

    Canvas->Pen->Color = clBlue;
    for (int i = 0; i < FBitCount - 1; i++) {
        int x1 = FBounds.Left + i * bitWidth + bitWidth - 2;
        int x2 = x1 + 2;
        int y = (FBounds.Top + FBounds.Bottom) / 2;
        Canvas->MoveTo(x1, y);
        Canvas->LineTo(x2, y);
    }
    Canvas->Pen->Color = clBlack;

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "SR");

    DrawConnectionPoints(Canvas);
}

void TShiftRegister::SaveToIni(TIniFile* IniFile, const String& Section) const {
    TCircuitElement::SaveToIni(IniFile, Section);
    IniFile->WriteInteger(Section, "BitCount", FBitCount);
}

void TShiftRegister::LoadFromIni(TIniFile* IniFile, const String& Section) {
    TCircuitElement::LoadFromIni(IniFile, Section);
    FBitCount = IniFile->ReadInteger(Section, "BitCount", 4);
}
void TCircuitElement::SaveToIni(TIniFile* IniFile, const String& Section) const {
    IniFile->WriteString(Section, "ClassName", GetClassName());
    IniFile->WriteString(Section, "Name", FName);
    IniFile->WriteInteger(Section, "Id", FId);
    IniFile->WriteInteger(Section, "X", FBounds.Left);
    IniFile->WriteInteger(Section, "Y", FBounds.Top);
    IniFile->WriteInteger(Section, "Width", FBounds.Width());
    IniFile->WriteInteger(Section, "Height", FBounds.Height());

    int stateValue = static_cast<int>(FCurrentState);
    IniFile->WriteInteger(Section, "CurrentState", stateValue);

    IniFile->WriteInteger(Section, "InputCount", FInputs.size());
    for (int i = 0; i < FInputs.size(); i++) {
        String inputSection = Section + "_Input_" + IntToStr(i);
        IniFile->WriteFloat(inputSection, "RelX", FInputs[i].RelX);
        IniFile->WriteFloat(inputSection, "RelY", FInputs[i].RelY);
        IniFile->WriteInteger(inputSection, "LineStyle", static_cast<int>(FInputs[i].LineStyle));
    }

    IniFile->WriteInteger(Section, "OutputCount", FOutputs.size());
    for (int i = 0; i < FOutputs.size(); i++) {
        String outputSection = Section + "_Output_" + IntToStr(i);
        IniFile->WriteFloat(outputSection, "RelX", FOutputs[i].RelX);
        IniFile->WriteFloat(outputSection, "RelY", FOutputs[i].RelY);
        IniFile->WriteInteger(outputSection, "LineStyle", static_cast<int>(FOutputs[i].LineStyle));
    }
}

void TCircuitElement::LoadFromIni(TIniFile* IniFile, const String& Section) {
    FName = IniFile->ReadString(Section, "Name", "");
    FId = IniFile->ReadInteger(Section, "Id", 0);

    int x = IniFile->ReadInteger(Section, "X", 0);
    int y = IniFile->ReadInteger(Section, "Y", 0);
    int width = IniFile->ReadInteger(Section, "Width", 80);
    int height = IniFile->ReadInteger(Section, "Height", 60);

    FBounds = TRect(x, y, x + width, y + height);

    int stateValue = IniFile->ReadInteger(Section, "CurrentState", 0);
    FCurrentState = static_cast<TTernary>(stateValue);

    int inputCount = IniFile->ReadInteger(Section, "InputCount", 0);
    FInputs.clear();
    for (int i = 0; i < inputCount; i++) {
        String inputSection = Section + "_Input_" + IntToStr(i);
        double relX = IniFile->ReadFloat(inputSection, "RelX", 0);
        double relY = IniFile->ReadFloat(inputSection, "RelY", 0);
        TLineStyle lineStyle = static_cast<TLineStyle>(IniFile->ReadInteger(inputSection, "LineStyle", 0));

        int absX = FBounds.Left + (int)(relX * FBounds.Width());
        int absY = FBounds.Top + (int)(relY * FBounds.Height());

        FInputs.push_back(TConnectionPoint(this, absX, absY, TTernary::ZERO, true, lineStyle));
        FInputs.back().RelX = relX;
        FInputs.back().RelY = relY;
    }

    int outputCount = IniFile->ReadInteger(Section, "OutputCount", 0);
    FOutputs.clear();
    for (int i = 0; i < outputCount; i++) {
        String outputSection = Section + "_Output_" + IntToStr(i);
        double relX = IniFile->ReadFloat(outputSection, "RelX", 0);
        double relY = IniFile->ReadFloat(outputSection, "RelY", 0);
        TLineStyle lineStyle = static_cast<TLineStyle>(IniFile->ReadInteger(outputSection, "LineStyle", 0));

        int absX = FBounds.Left + (int)(relX * FBounds.Width());
        int absY = FBounds.Top + (int)(relY * FBounds.Height());

        FOutputs.push_back(TConnectionPoint(this, absX, absY, TTernary::ZERO, false, lineStyle));
        FOutputs.back().RelX = relX;
        FOutputs.back().RelY = relY;
    }
}
