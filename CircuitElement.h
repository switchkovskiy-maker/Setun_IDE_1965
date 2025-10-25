#ifndef CircuitElementH
#define CircuitElementH

#include "TernaryTypes.h"
#include <Vcl.Graphics.hpp>
#include <System.Types.hpp>
#include <vector>

class TCircuitElement {
protected:
    int FId;
    String FName;
    TRect FBounds;
    TElementType FType;
    std::vector<TConnectionPoint> FInputs;
    std::vector<TConnectionPoint> FOutputs;
    TTernary FCurrentState;
    
    // Вспомогательные методы для рисования по книжным стандартам
    void DrawMagneticAmplifier(TCanvas* Canvas, bool IsPowerful);
    void DrawTernaryElement(TCanvas* Canvas);
    void DrawControlLine(TCanvas* Canvas, const TConnectionPoint& Point);
    void DrawCrossingLine(TCanvas* Canvas, int X1, int Y1, int X2, int Y2);
    void DrawConnectionPoints(TCanvas* Canvas);
    
public:
    TCircuitElement(int AId, const String& AName, TElementType AType, int X, int Y);
    virtual ~TCircuitElement() {}
    
    virtual void Calculate() = 0;
    virtual void Draw(TCanvas* Canvas);
    virtual TConnectionPoint* GetConnectionAt(int X, int Y);
    
    // Метод SetBounds теперь определен inline в заголовочном файле
    void SetBounds(const TRect& NewBounds) { 
        // Сохраняем старые границы для вычисления относительных позиций
        TRect oldBounds = FBounds;
        FBounds = NewBounds; 
        
        // Пересчитываем абсолютные координаты на основе относительных
        for (auto& input : FInputs) {
            input.X = NewBounds.Left + (int)(input.RelX * NewBounds.Width());
            input.Y = NewBounds.Top + (int)(input.RelY * NewBounds.Height());
        }
        for (auto& output : FOutputs) {
            output.X = NewBounds.Left + (int)(output.RelX * NewBounds.Width());
            output.Y = NewBounds.Top + (int)(output.RelY * NewBounds.Height());
        }
    }
    
    // Метод для вычисления относительных позиций точек соединения
    void CalculateRelativePositions() {
        for (auto& input : FInputs) {
            if (FBounds.Width() > 0 && FBounds.Height() > 0) {
                input.RelX = (double)(input.X - FBounds.Left) / FBounds.Width();
                input.RelY = (double)(input.Y - FBounds.Top) / FBounds.Height();
            }
        }
        for (auto& output : FOutputs) {
            if (FBounds.Width() > 0 && FBounds.Height() > 0) {
                output.RelX = (double)(output.X - FBounds.Left) / FBounds.Width();
                output.RelY = (double)(output.Y - FBounds.Top) / FBounds.Height();
            }
        }
    }
    
    // Добавленные методы для доступа к protected членам
    void SetId(int AId) { FId = AId; }
    void SetName(const String& AName) { FName = AName; }
    void SetCurrentState(TTernary State) { FCurrentState = State; }
    
    // Свойства
    __property int Id = { read = FId };
    __property String Name = { read = FName };
    __property TRect Bounds = { read = FBounds };
    __property TElementType ElementType = { read = FType };
    __property TTernary CurrentState = { read = FCurrentState };
    __property std::vector<TConnectionPoint> Inputs = { read = FInputs };
    __property std::vector<TConnectionPoint> Outputs = { read = FOutputs };
};

// Магнитный усилитель (стр. 43-45, рис. 4)
class TMagneticAmplifier : public TCircuitElement {
private:
    bool FIsPowered;
    
public:
    TMagneticAmplifier(int AId, int X, int Y, bool IsPowerful = false);
    void Calculate() override;
};

// Троичный элемент (стр. 50-51, рис. 9)
class TTernaryElement : public TCircuitElement {
public:
    TTernaryElement(int AId, int X, int Y);
    void Calculate() override;
};

// Сдвигающий регистр (стр. 45-46, рис. 5)
class TShiftRegister : public TCircuitElement {
private:
    int FBitCount;
    
public:
    TShiftRegister(int AId, int X, int Y, int BitCount = 4);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

#endif