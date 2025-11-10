#ifndef CircuitElementH
#define CircuitElementH

#include "TernaryTypes.h"
#include <Vcl.Graphics.hpp>
#include <System.Types.hpp>
#include <vector>
#include <System.Classes.hpp>
#include <System.IniFiles.hpp>

class TCircuitElement {
protected:
    int FId;
    String FName;
    TRect FBounds;
    std::vector<TConnectionPoint> FInputs;
    std::vector<TConnectionPoint> FOutputs;
    TTernary FCurrentState;

    void DrawMagneticAmplifier(TCanvas* Canvas, bool IsPowerful);
    void DrawTernaryElement(TCanvas* Canvas);
    void DrawControlLine(TCanvas* Canvas, const TConnectionPoint& Point);
    void DrawCrossingLine(TCanvas* Canvas, int X1, int Y1, int X2, int Y2);
    void DrawConnectionPoints(TCanvas* Canvas);

public:
    TCircuitElement(int AId, const String& AName, int X, int Y);
    virtual ~TCircuitElement() {}

    virtual void Calculate() { /* Базовая реализация - ничего не делает */ }
    virtual void Draw(TCanvas* Canvas);
    virtual TConnectionPoint* GetConnectionAt(int X, int Y);

    void SetBounds(const TRect& NewBounds) {
        FBounds = NewBounds;

        for (auto& input : FInputs) {
            input.X = NewBounds.Left + (int)(input.RelX * NewBounds.Width());
            input.Y = NewBounds.Top + (int)(input.RelY * NewBounds.Height());
        }
        for (auto& output : FOutputs) {
            output.X = NewBounds.Left + (int)(output.RelX * NewBounds.Width());
            output.Y = NewBounds.Top + (int)(output.RelY * NewBounds.Height());
        }
    }

    void CalculateRelativePositions() {
        for (auto& input : FInputs) {
            if (FBounds.Width() > 0 && FBounds.Height() > 0) {
                input.RelX = (double)(input.X - FBounds.Left) / FBounds.Width();
                input.RelY = (double)(input.Y - FBounds.Top) / FBounds.Height();
            } else {
                input.RelX = 0.0;
                input.RelY = 0.5;
            }
        }
        for (auto& output : FOutputs) {
            if (FBounds.Width() > 0 && FBounds.Height() > 0) {
                output.RelX = (double)(output.X - FBounds.Left) / FBounds.Width();
                output.RelY = (double)(output.Y - FBounds.Top) / FBounds.Height();
            } else {
                output.RelX = 1.0;
                output.RelY = 0.5;
            }
        }
    }

    void SetId(int AId) { FId = AId; }
    void SetName(const String& AName) { FName = AName; }
    void SetCurrentState(TTernary State) { FCurrentState = State; }

    TConnectionPoint* FindConnectionPointByRelPos(double relX, double relY, bool IsInput) {
        auto& points = IsInput ? FInputs : FOutputs;
        for (auto& point : points) {
            if (fabs(point.RelX - relX) < 0.01 && fabs(point.RelY - relY) < 0.01) {
                return &point;
            }
        }
        return nullptr;
    }

    virtual String GetClassName() const { return "TCircuitElement"; }
    virtual void SaveToIni(TIniFile* IniFile, const String& Section) const;
    virtual void LoadFromIni(TIniFile* IniFile, const String& Section);

    __property int Id = { read = FId };
    __property String Name = { read = FName };
    __property TRect Bounds = { read = FBounds };
    __property TTernary CurrentState = { read = FCurrentState };
    __property std::vector<TConnectionPoint> Inputs = { read = FInputs };
    __property std::vector<TConnectionPoint> Outputs = { read = FOutputs };
};

class TMagneticAmplifier : public TCircuitElement {
private:
    bool FIsPowered;

public:
    TMagneticAmplifier(int AId, int X, int Y, bool IsPowerful = false);
    void Calculate() override;
    virtual String GetClassName() const override { return "TMagneticAmplifier"; }
};

class TTernaryElement : public TCircuitElement {
public:
    TTernaryElement(int AId, int X, int Y);
    void Calculate() override;
    virtual String GetClassName() const override { return "TTernaryElement"; }
};

class TShiftRegister : public TCircuitElement {
private:
    int FBitCount;

public:
    TShiftRegister(int AId, int X, int Y, int BitCount = 4);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    virtual String GetClassName() const override { return "TShiftRegister"; }
    virtual void SaveToIni(TIniFile* IniFile, const String& Section) const override;
    virtual void LoadFromIni(TIniFile* IniFile, const String& Section) override;
};

#endif
