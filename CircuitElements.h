#ifndef CircuitElementsH
#define CircuitElementsH

#include "CircuitElement.h"

// Троичный триггер (стр. 55-56, рис. 10)
class TTernaryTrigger : public TCircuitElement {
private:
    TTernary FStoredState;
    
public:
    TTernaryTrigger(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    void SetState(TTernary State);
    void Reset();
};

// Полусумматор (стр. 58, рис. 13)
class THalfAdder : public TCircuitElement {
public:
    THalfAdder(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

// Троичный сумматор (стр. 58-59, рис. 14)
class TTernaryAdder : public TCircuitElement {
public:
    TTernaryAdder(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

// Дешифратор кода (стр. 57, рис. 12)
class TDecoder : public TCircuitElement {
private:
    int FInputBits;
    int FOutputCount;
    
public:
    TDecoder(int AId, int X, int Y, int InputBits = 2);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

// Троичный счетчик (стр. 59, рис. 15)
class TCounter : public TCircuitElement {
private:
    int FCount;
    int FMaxCount;
    
public:
    TCounter(int AId, int X, int Y, int BitCount = 2);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    void Reset();
};

// Распределитель импульсов (стр. 56, рис. 11)
class TDistributor : public TCircuitElement {
private:
    int FCurrentStep;
    int FTotalSteps;
    
public:
    TDistributor(int AId, int X, int Y, int Steps = 8);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    void AdvanceStep();
};

// Переключатель (стр. 55, рис. 10)
class TSwitch : public TCircuitElement {
private:
    int FSelectedOutput;
    
public:
    TSwitch(int AId, int X, int Y, int OutputCount = 3);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    void SetSelection(int OutputIndex);
};

// Логический элемент И (стр. 49-50, рис. 8)
class TLogicAnd : public TCircuitElement {
public:
    TLogicAnd(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

// Логический элемент ИЛИ (стр. 49, рис. 7)
class TLogicOr : public TCircuitElement {
public:
    TLogicOr(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

// Схема запрета (стр. 47-48, рис. 6)
class TLogicInhibit : public TCircuitElement {
public:
    TLogicInhibit(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

// Генератор единиц (стр. 50, рис. 8)
class TGenerator : public TCircuitElement {
public:
    TGenerator(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

#endif