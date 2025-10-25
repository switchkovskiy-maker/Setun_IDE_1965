#ifndef CircuitElementsH
#define CircuitElementsH

#include "CircuitElement.h"

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

class THalfAdder : public TCircuitElement {
public:
    THalfAdder(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

class TTernaryAdder : public TCircuitElement {
public:
    TTernaryAdder(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

class TDecoder : public TCircuitElement {
private:
    int FInputBits;
    int FOutputCount;
    
public:
    TDecoder(int AId, int X, int Y, int InputBits = 2);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

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

class TSwitch : public TCircuitElement {
private:
    int FSelectedOutput;
    
public:
    TSwitch(int AId, int X, int Y, int OutputCount = 3);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    void SetSelection(int OutputIndex);
};

class TLogicAnd : public TCircuitElement {
public:
    TLogicAnd(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

class TLogicOr : public TCircuitElement {
public:
    TLogicOr(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

class TLogicInhibit : public TCircuitElement {
public:
    TLogicInhibit(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

class TGenerator : public TCircuitElement {
public:
    TGenerator(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
};

#endif