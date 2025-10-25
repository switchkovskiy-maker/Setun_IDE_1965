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
    void DrawConnectionPoints(TCanvas* Canvas); // Добавлено объявление

public:
    TCircuitElement(int AId, const String& AName, TElementType AType, int X, int Y);
    virtual ~TCircuitElement() {}

    virtual void Calculate() = 0;
    virtual void Draw(TCanvas* Canvas);
    virtual TConnectionPoint* GetConnectionAt(int X, int Y);

    // Метод SetBounds теперь определен inline в заголовочном файле
    void SetBounds(const TRect& NewBounds) {
        // Сохраняем смещения для корректного обновления соединений
        int deltaX = NewBounds.Left - FBounds.Left;
        int deltaY = NewBounds.Top - FBounds.Top;

        FBounds = NewBounds;

        // Обновляем позиции всех входов и выходов
        for (auto& input : FInputs) {
            input.X += deltaX;
            input.Y += deltaY;
        }
        for (auto& output : FOutputs) {
            output.X += deltaX;
            output.Y += deltaY;
        }
    }

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
