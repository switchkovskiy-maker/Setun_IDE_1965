#ifndef CircuitElementsH
#define CircuitElementsH

#include "CircuitElement.h"

// Все специализированные элементы теперь определяются здесь
// Они будут регистрироваться в библиотеках через фабрики

class TTernaryTrigger : public TCircuitElement {
private:
    TTernary FStoredState;

public:
    TTernaryTrigger(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    void SetState(TTernary State);
    void Reset();
    virtual String GetClassName() const override { return "TTernaryTrigger"; }
    virtual void SaveToIni(TIniFile* IniFile, const String& Section) const override;
    virtual void LoadFromIni(TIniFile* IniFile, const String& Section) override;
};

class THalfAdder : public TCircuitElement {
public:
    THalfAdder(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    virtual String GetClassName() const override { return "THalfAdder"; }
};

class TTernaryAdder : public TCircuitElement {
public:
    TTernaryAdder(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    virtual String GetClassName() const override { return "TTernaryAdder"; }
};

class TDecoder : public TCircuitElement {
private:
    int FInputBits;
    int FOutputCount;

public:
    TDecoder(int AId, int X, int Y, int InputBits = 2);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    virtual String GetClassName() const override { return "TDecoder"; }
    virtual void SaveToIni(TIniFile* IniFile, const String& Section) const override;
    virtual void LoadFromIni(TIniFile* IniFile, const String& Section) override;
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
    virtual String GetClassName() const override { return "TCounter"; }
    virtual void SaveToIni(TIniFile* IniFile, const String& Section) const override;
    virtual void LoadFromIni(TIniFile* IniFile, const String& Section) override;
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
    virtual String GetClassName() const override { return "TDistributor"; }
    virtual void SaveToIni(TIniFile* IniFile, const String& Section) const override;
    virtual void LoadFromIni(TIniFile* IniFile, const String& Section) override;
};

class TSwitch : public TCircuitElement {
private:
    int FSelectedOutput;

public:
    TSwitch(int AId, int X, int Y, int OutputCount = 3);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    void SetSelection(int OutputIndex);
    virtual String GetClassName() const override { return "TSwitch"; }
    virtual void SaveToIni(TIniFile* IniFile, const String& Section) const override;
    virtual void LoadFromIni(TIniFile* IniFile, const String& Section) override;
};

class TLogicAnd : public TCircuitElement {
public:
    TLogicAnd(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    virtual String GetClassName() const override { return "TLogicAnd"; }
};

class TLogicOr : public TCircuitElement {
public:
    TLogicOr(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    virtual String GetClassName() const override { return "TLogicOr"; }
};

class TLogicInhibit : public TCircuitElement {
public:
    TLogicInhibit(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    virtual String GetClassName() const override { return "TLogicInhibit"; }
};

class TGenerator : public TCircuitElement {
public:
    TGenerator(int AId, int X, int Y);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    virtual String GetClassName() const override { return "TGenerator"; }
};

#endif