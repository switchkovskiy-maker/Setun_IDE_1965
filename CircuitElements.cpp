#include "CircuitElements.h"
#include <Vcl.Graphics.hpp>
#include <math.h>

#pragma package(smart_init)

// TTernaryTrigger
TTernaryTrigger::TTernaryTrigger(int AId, int X, int Y)
    : TCircuitElement(AId, "Троичный триггер", X, Y),
      FStoredState(TTernary::ZERO) {

    FBounds = TRect(X, Y, X + 120, Y + 80);

    FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+60, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));

    FOutputs.push_back(TConnectionPoint(this, X+135, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    FOutputs.push_back(TConnectionPoint(this, X+135, Y+50, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TTernaryTrigger::Calculate() {
    if (FInputs.size() >= 3 && FOutputs.size() >= 2) {
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

        FOutputs[0].Value = FStoredState;
        FOutputs[1].Value = (FStoredState == TTernary::POS) ? TTernary::NEG :
                           (FStoredState == TTernary::NEG) ? TTernary::POS : TTernary::ZERO;
    }
}

void TTernaryTrigger::Draw(TCanvas* Canvas) {
    int centerX = (FBounds.Left + FBounds.Right) / 2;
    int centerY = (FBounds.Top + FBounds.Bottom) / 2;

    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;

    TRect elem1 = TRect(centerX - 40, centerY - 35, centerX - 5, centerY - 5);
    TRect elem2 = TRect(centerX + 5, centerY + 5, centerX + 40, centerY + 35);

    Canvas->Rectangle(elem1.Left, elem1.Top, elem1.Right, elem1.Bottom);
    Canvas->Pen->Width = 2;
    Canvas->Rectangle(elem2.Left, elem2.Top, elem2.Right, elem2.Bottom);
    Canvas->Pen->Width = 1;
    Canvas->Rectangle(elem2.Left+2, elem2.Top+2, elem2.Right-2, elem2.Bottom-2);

    Canvas->Pen->Color = clBlue;
    Canvas->MoveTo(elem1.Right, elem1.Top + 10);
    Canvas->LineTo(elem2.Left, elem2.Bottom - 10);
    Canvas->MoveTo(elem2.Left, elem2.Top + 10);
    Canvas->LineTo(elem1.Right, elem1.Bottom - 10);
    Canvas->Pen->Color = clBlack;

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->Font->Size = 7;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Trig");
    Canvas->TextOut(FInputs[0].X - 20, FInputs[0].Y - 5, "Set+");
    Canvas->TextOut(FInputs[1].X - 20, FInputs[1].Y - 5, "Set-");
    Canvas->TextOut(FInputs[2].X - 20, FInputs[2].Y - 5, "Rst");
    Canvas->TextOut(FOutputs[0].X + 5, FOutputs[0].Y - 5, "Q");
    Canvas->TextOut(FOutputs[1].X + 5, FOutputs[1].Y - 5, "Q~");

    DrawConnectionPoints(Canvas);
}

void TTernaryTrigger::SetState(TTernary State) {
    FStoredState = State;
}

void TTernaryTrigger::Reset() {
    FStoredState = TTernary::ZERO;
}

void TTernaryTrigger::SaveToIni(TIniFile* IniFile, const String& Section) const {
    TCircuitElement::SaveToIni(IniFile, Section);
    IniFile->WriteInteger(Section, "StoredState", static_cast<int>(FStoredState));
}

void TTernaryTrigger::LoadFromIni(TIniFile* IniFile, const String& Section) {
    TCircuitElement::LoadFromIni(IniFile, Section);
    int stateValue = IniFile->ReadInteger(Section, "StoredState", 0);
    FStoredState = static_cast<TTernary>(stateValue);
}

// THalfAdder
THalfAdder::THalfAdder(int AId, int X, int Y)
    : TCircuitElement(AId, "Полусумматор", X, Y) {

    FBounds = TRect(X, Y, X + 80, Y + 60);

    FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+95, Y+20, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    FOutputs.push_back(TConnectionPoint(this, X+95, Y+40, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void THalfAdder::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 2) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;

        if (a == TTernary::ZERO && b == TTernary::ZERO) {
            FOutputs[0].Value = TTernary::ZERO;
            FOutputs[1].Value = TTernary::ZERO;
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
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 10, FBounds.Top + 10, "Σ");
    Canvas->TextOut(FBounds.Left + 10, FBounds.Top + 30, "C");

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->TextOut(FInputs[0].X - 10, FInputs[0].Y - 5, "A");
    Canvas->TextOut(FInputs[1].X - 10, FInputs[1].Y - 5, "B");
    Canvas->TextOut(FOutputs[0].X + 5, FOutputs[0].Y - 5, "S");
    Canvas->TextOut(FOutputs[1].X + 5, FOutputs[1].Y - 5, "C");

    DrawConnectionPoints(Canvas);
}

// TTernaryAdder
TTernaryAdder::TTernaryAdder(int AId, int X, int Y)
    : TCircuitElement(AId, "Троичный сумматор", X, Y) {

    FBounds = TRect(X, Y, X + 100, Y + 80);

    FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+60, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+115, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    FOutputs.push_back(TConnectionPoint(this, X+115, Y+50, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TTernaryAdder::Calculate() {
    if (FInputs.size() >= 3 && FOutputs.size() >= 2) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;
        TTernary carryIn = FInputs[2].Value;

        int a_val = (a == TTernary::POS) ? 1 : (a == TTernary::NEG) ? -1 : 0;
        int b_val = (b == TTernary::POS) ? 1 : (b == TTernary::NEG) ? -1 : 0;
        int c_val = (carryIn == TTernary::POS) ? 1 : (carryIn == TTernary::NEG) ? -1 : 0;

        int sum = a_val + b_val + c_val;

        if (sum >= 2) {
            FOutputs[0].Value = TTernary::POS;
            FOutputs[1].Value = TTernary::POS;
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
    int centerX = (FBounds.Left + FBounds.Right) / 2;

    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;

    TRect ha1 = TRect(FBounds.Left + 10, FBounds.Top + 10, centerX - 5, FBounds.Top + 40);
    TRect ha2 = TRect(centerX + 5, FBounds.Top + 30, FBounds.Right - 10, FBounds.Top + 60);

    Canvas->Rectangle(ha1.Left, ha1.Top, ha1.Right, ha1.Bottom);
    Canvas->Rectangle(ha2.Left, ha2.Top, ha2.Right, ha2.Bottom);

    Canvas->Pen->Color = clBlue;
    Canvas->MoveTo(ha1.Right, ha1.Top + 15);
    Canvas->LineTo(ha2.Left, ha2.Top + 15);
    Canvas->MoveTo(ha1.Right, ha1.Top + 25);
    Canvas->LineTo(ha2.Left, ha2.Bottom - 10);
    Canvas->Pen->Color = clBlack;

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->Font->Size = 7;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "TAdd");
    Canvas->TextOut(FInputs[0].X - 10, FInputs[0].Y - 5, "A");
    Canvas->TextOut(FInputs[1].X - 10, FInputs[1].Y - 5, "B");
    Canvas->TextOut(FInputs[2].X - 10, FInputs[2].Y - 5, "Cin");
    Canvas->TextOut(FOutputs[0].X + 5, FOutputs[0].Y - 5, "S");
    Canvas->TextOut(FOutputs[1].X + 5, FOutputs[1].Y - 5, "Cout");

    DrawConnectionPoints(Canvas);
}

// TDecoder
TDecoder::TDecoder(int AId, int X, int Y, int InputBits)
    : TCircuitElement(AId, "Дешифратор", X, Y),
      FInputBits(InputBits), FOutputCount((int)pow(3, InputBits)) {

    FBounds = TRect(X, Y, X + 80, Y + 30 + FOutputCount * 15);

    for (int i = 0; i < FInputBits; i++) {
        FInputs.push_back(TConnectionPoint(this, X-15, Y+20 + i*15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    }

    for (int i = 0; i < FOutputCount; i++) {
        FOutputs.push_back(TConnectionPoint(this, X+95, Y+15 + i*15, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    }

    CalculateRelativePositions();
}

void TDecoder::Calculate() {
    int activeOutput = 0;
    for (int i = 0; i < FInputBits; i++) {
        if (i < FInputs.size()) {
            int digitValue = (FInputs[i].Value == TTernary::POS) ? 2 :
                           (FInputs[i].Value == TTernary::ZERO) ? 1 : 0;
            activeOutput = activeOutput * 3 + digitValue;
        }
    }

    for (int i = 0; i < FOutputs.size(); i++) {
        FOutputs[i].Value = (i == activeOutput) ? TTernary::POS : TTernary::ZERO;
    }
}

void TDecoder::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }

    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->Font->Size = 7;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Dec");

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

    DrawConnectionPoints(Canvas);
}

void TDecoder::SaveToIni(TIniFile* IniFile, const String& Section) const {
    TCircuitElement::SaveToIni(IniFile, Section);
    IniFile->WriteInteger(Section, "InputBits", FInputBits);
}

void TDecoder::LoadFromIni(TIniFile* IniFile, const String& Section) {
    TCircuitElement::LoadFromIni(IniFile, Section);
    FInputBits = IniFile->ReadInteger(Section, "InputBits", 2);
    FOutputCount = static_cast<int>(pow(3, FInputBits));
}

// TCounter
TCounter::TCounter(int AId, int X, int Y, int BitCount)
    : TCircuitElement(AId, "Счетчик", X, Y),
      FCount(0), FMaxCount((int)pow(3, BitCount) - 1) {

    FBounds = TRect(X, Y, X + 40 + BitCount * 25, Y + 60);

    FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+FBounds.Width()+5, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TCounter::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
        TTernary inc = FInputs[0].Value;
        TTernary dec = FInputs[1].Value;

        if (inc == TTernary::POS) {
            FCount = (FCount < FMaxCount) ? FCount + 1 : 0;
        } else if (dec == TTernary::POS) {
            FCount = (FCount > 0) ? FCount - 1 : FMaxCount;
        }

        FOutputs[0].Value = (FCount > FMaxCount/2) ? TTernary::POS :
                           (FCount < FMaxCount/2) ? TTernary::NEG : TTernary::ZERO;
    }
}

void TCounter::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;

    int bitWidth = 25;
    for (int i = 0; i < (FMaxCount + 1); i++) {
        int left = FBounds.Left + i * bitWidth;
        TRect bitRect = TRect(left, FBounds.Top + 10, left + bitWidth - 2, FBounds.Bottom - 10);

        if (i == FCount) {
            Canvas->Brush->Color = clYellow;
            Canvas->Rectangle(bitRect.Left, bitRect.Top, bitRect.Right, bitRect.Bottom);
            Canvas->Brush->Color = clWhite;
        } else {
            Canvas->Rectangle(bitRect.Left, bitRect.Top, bitRect.Right, bitRect.Bottom);
        }

        Canvas->Font->Size = 6;
        Canvas->TextOut(bitRect.Left + 2, bitRect.Top + 2, IntToStr(i));
    }

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Cnt");
    Canvas->TextOut(FInputs[0].X - 10, FInputs[0].Y - 5, "+1");
    Canvas->TextOut(FInputs[1].X - 10, FInputs[1].Y - 5, "-1");

    DrawConnectionPoints(Canvas);
}

void TCounter::Reset() {
    FCount = 0;
}

void TCounter::SaveToIni(TIniFile* IniFile, const String& Section) const {
    TCircuitElement::SaveToIni(IniFile, Section);
    IniFile->WriteInteger(Section, "Count", FCount);
    IniFile->WriteInteger(Section, "MaxCount", FMaxCount);
}

void TCounter::LoadFromIni(TIniFile* IniFile, const String& Section) {
    TCircuitElement::LoadFromIni(IniFile, Section);
    FCount = IniFile->ReadInteger(Section, "Count", 0);
    FMaxCount = IniFile->ReadInteger(Section, "MaxCount", static_cast<int>(pow(3, 2) - 1));
}

// TDistributor
TDistributor::TDistributor(int AId, int X, int Y, int Steps)
    : TCircuitElement(AId, "Distributor", X, Y),
      FCurrentStep(0), FTotalSteps(Steps) {

    FBounds = TRect(X, Y, X + 80, Y + 60);

    FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));

    for (int i = 0; i < Steps; i++) {
        FOutputs.push_back(TConnectionPoint(this, X+95, Y+15 + i*10, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    }

    CalculateRelativePositions();
}

void TDistributor::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= FTotalSteps) {
        TTernary clock = FInputs[0].Value;
        TTernary reset = FInputs[1].Value;

        if (reset == TTernary::POS) {
            FCurrentStep = 0;
        } else if (clock == TTernary::POS) {
            FCurrentStep = (FCurrentStep + 1) % FTotalSteps;
        }

        for (int i = 0; i < FOutputs.size(); i++) {
            FOutputs[i].Value = (i == FCurrentStep) ? TTernary::POS : TTernary::ZERO;
        }
    }
}

void TDistributor::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    int centerX = (FBounds.Left + FBounds.Right) / 2;
    int centerY = (FBounds.Top + FBounds.Bottom) / 2;

    Canvas->Ellipse(centerX - 25, centerY - 25, centerX + 25, centerY + 25);

    double angle = 2 * M_PI * FCurrentStep / FTotalSteps;
    int markerX = centerX + (int)(20 * cos(angle));
    int markerY = centerY + (int)(20 * sin(angle));

    Canvas->Brush->Color = clRed;
    Canvas->Ellipse(markerX - 3, markerY - 3, markerX + 3, markerY + 3);
    Canvas->Brush->Color = clWhite;

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Dist");
    Canvas->TextOut(FInputs[0].X - 10, FInputs[0].Y - 5, "Clk");
    Canvas->TextOut(FInputs[1].X - 10, FInputs[1].Y - 5, "Rst");

    for (int i = 0; i < FOutputs.size(); i++) {
        Canvas->TextOut(FOutputs[i].X + 5, FOutputs[i].Y - 5, IntToStr(i));
    }

    DrawConnectionPoints(Canvas);
}

void TDistributor::AdvanceStep() {
    FCurrentStep = (FCurrentStep + 1) % FTotalSteps;
}

void TDistributor::SaveToIni(TIniFile* IniFile, const String& Section) const {
    TCircuitElement::SaveToIni(IniFile, Section);
    IniFile->WriteInteger(Section, "CurrentStep", FCurrentStep);
    IniFile->WriteInteger(Section, "TotalSteps", FTotalSteps);
}

void TDistributor::LoadFromIni(TIniFile* IniFile, const String& Section) {
    TCircuitElement::LoadFromIni(IniFile, Section);
    FCurrentStep = IniFile->ReadInteger(Section, "CurrentStep", 0);
    FTotalSteps = IniFile->ReadInteger(Section, "TotalSteps", 8);
}

// TSwitch
TSwitch::TSwitch(int AId, int X, int Y, int OutputCount)
    : TCircuitElement(AId, "Switch", X, Y),
      FSelectedOutput(0) {

    FBounds = TRect(X, Y, X + 60, Y + 40);

    FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));

    for (int i = 0; i < OutputCount; i++) {
        FOutputs.push_back(TConnectionPoint(this, X+75, Y+10 + i*10, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    }

    CalculateRelativePositions();
}

void TSwitch::Calculate() {
    if (FInputs.size() >= 1 && FOutputs.size() > FSelectedOutput) {
        TTernary inputValue = FInputs[0].Value;

        for (int i = 0; i < FOutputs.size(); i++) {
            FOutputs[i].Value = (i == FSelectedOutput) ? inputValue : TTernary::ZERO;
        }
    }
}

void TSwitch::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    int centerX = (FBounds.Left + FBounds.Right) / 2;
    int centerY = (FBounds.Top + FBounds.Bottom) / 2;

    Canvas->MoveTo(FBounds.Left + 10, centerY);
    Canvas->LineTo(FBounds.Right - 10, centerY);

    int leverX = centerX;
    int leverY = centerY - 15;
    Canvas->MoveTo(centerX, centerY);
    Canvas->LineTo(leverX, leverY);

    double angle = -M_PI/4 + (M_PI/2 * FSelectedOutput / (FOutputs.size() - 1));
    int endX = centerX + (int)(12 * sin(angle));
    int endY = centerY - (int)(12 * cos(angle));

    Canvas->MoveTo(centerX, centerY);
    Canvas->LineTo(endX, endY);

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Switch");
    Canvas->TextOut(FInputs[0].X - 10, FInputs[0].Y - 5, "In");

    for (int i = 0; i < FOutputs.size(); i++) {
        Canvas->TextOut(FOutputs[i].X + 5, FOutputs[i].Y - 5, IntToStr(i));
    }

    DrawConnectionPoints(Canvas);
}

void TSwitch::SetSelection(int OutputIndex) {
    if (OutputIndex >= 0 && OutputIndex < FOutputs.size()) {
        FSelectedOutput = OutputIndex;
    }
}

void TSwitch::SaveToIni(TIniFile* IniFile, const String& Section) const {
    TCircuitElement::SaveToIni(IniFile, Section);
    IniFile->WriteInteger(Section, "SelectedOutput", FSelectedOutput);
}

void TSwitch::LoadFromIni(TIniFile* IniFile, const String& Section) {
    TCircuitElement::LoadFromIni(IniFile, Section);
    FSelectedOutput = IniFile->ReadInteger(Section, "SelectedOutput", 0);
}

// TLogicAnd
TLogicAnd::TLogicAnd(int AId, int X, int Y)
    : TCircuitElement(AId, "Логическое И", X, Y) {

    FBounds = TRect(X, Y, X + 60, Y + 40);
    FInputs.push_back(TConnectionPoint(this, X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+25, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+75, Y+20, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TLogicAnd::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;

        FOutputs[0].Value = (a == TTernary::POS && b == TTernary::POS) ? TTernary::POS : TTernary::ZERO;
    }
}

void TLogicAnd::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    Canvas->Font->Size = 10;
    Canvas->TextOut(FBounds.Left + 20, FBounds.Top + 10, "&");

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    DrawConnectionPoints(Canvas);
}

// TLogicOr
TLogicOr::TLogicOr(int AId, int X, int Y)
    : TCircuitElement(AId, "Логическое ИЛИ", X, Y) {

    FBounds = TRect(X, Y, X + 60, Y + 40);
    FInputs.push_back(TConnectionPoint(this, X-15, Y+15, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+25, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+75, Y+20, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TLogicOr::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;

        FOutputs[0].Value = (a == TTernary::POS || b == TTernary::POS) ? TTernary::POS : TTernary::ZERO;
    }
}

void TLogicOr::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    Canvas->Font->Size = 10;
    Canvas->TextOut(FBounds.Left + 20, FBounds.Top + 10, "≥1");

    for (const auto& input : FInputs) {
        DrawControlLine(Canvas, input);
    }
    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    DrawConnectionPoints(Canvas);
}

// TLogicInhibit
TLogicInhibit::TLogicInhibit(int AId, int X, int Y)
    : TCircuitElement(AId, "Запрет", X, Y) {

    FBounds = TRect(X, Y, X + 80, Y + 60);
    FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::NEGATIVE_CONTROL));
    FOutputs.push_back(TConnectionPoint(this, X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TLogicInhibit::Calculate() {
    if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
        TTernary a = FInputs[0].Value;
        TTernary b = FInputs[1].Value;

        FOutputs[0].Value = (b == TTernary::ZERO) ? a : TTernary::ZERO;
    }
}

void TLogicInhibit::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

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

    DrawConnectionPoints(Canvas);
}

// TGenerator
TGenerator::TGenerator(int AId, int X, int Y)
    : TCircuitElement(AId, "Генератор", X, Y) {

    FBounds = TRect(X, Y, X + 50, Y + 30);
    FOutputs.push_back(TConnectionPoint(this, X+65, Y+15, TTernary::POS, false, TLineStyle::OUTPUT_LINE));

    CalculateRelativePositions();
}

void TGenerator::Calculate() {
    if (FOutputs.size() >= 1) {
        FOutputs[0].Value = TTernary::POS;
    }
}

void TGenerator::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clBlack;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

    Canvas->Font->Size = 8;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "Gen");
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 20, "1");

    for (const auto& output : FOutputs) {
        DrawControlLine(Canvas, output);
    }

    DrawConnectionPoints(Canvas);
}
