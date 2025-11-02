#define BUILD_DLL

#include "EmulatorSetun1958.h"
#include "CircuitElements.h"
#include <memory>
#include <stdexcept>

// Включаем исправленные заголовочные файлы
#include "types_tte.h"
#include "setun_tte.h"

// Глобальный указатель на библиотеку
TComponentLibrary* GEmulatorSetun1958Library = nullptr;

// ============================================================================
// КЛАССЫ-АДАПТЕРЫ ДЛЯ ЭЛЕМЕНТОВ СЕТУНЬ-1958
// ============================================================================

// Базовый класс для всех элементов Сетунь-1958
class TSetunElement : public TCircuitElement {
protected:
    void DrawSetunSymbol(TCanvas* Canvas, const String& Symbol) {
        Canvas->Brush->Color = clWhite;
        Canvas->Pen->Color = clBlack;
        Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);

        Canvas->Font->Size = 8;
        Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, Symbol);

        DrawConnectionPoints(Canvas);
    }

public:
    TSetunElement(int AId, const String& AName, int X, int Y)
        : TCircuitElement(AId, AName, X, Y) {}
};

// Элемент BTE-1: Y5 = A * NOT(B)
class TBTE1Element : public TSetunElement {
private:
    bte_1_st_t bte1_state;

public:
    TBTE1Element(int AId, int X, int Y)
        : TSetunElement(AId, "BTE-1", X, Y) {

        FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FOutputs.push_back(TConnectionPoint(this, X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

        CalculateRelativePositions();
        // Инициализация состояния
        bte1_state.a1 = 0;
        bte1_state.b1 = 0;
        bte1_state.y5 = 0;
        bte1_state.fc1 = bte1_state.fc2 = bte1_state.fc3 = 0;
    }

    void Calculate() override {
        if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
            S8 a = (FInputs[0].Value == TTernary::POS) ? 1 : 0;
            S8 b = (FInputs[1].Value == TTernary::POS) ? 1 : 0;

            bte_1_fn(a, b, &bte1_state);

            FOutputs[0].Value = (bte1_state.y5 == 1) ? TTernary::POS : TTernary::ZERO;
        }
    }

    void Draw(TCanvas* Canvas) override {
        DrawSetunSymbol(Canvas, "BTE-1");
    }

    virtual String GetClassName() const override { return "TBTE1Element"; }
};

// Элемент BTE-2: Y5 = A * B
class TBTE2Element : public TSetunElement {
private:
    bte_2_st_t bte2_state;

public:
    TBTE2Element(int AId, int X, int Y)
        : TSetunElement(AId, "BTE-2", X, Y) {

        FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FOutputs.push_back(TConnectionPoint(this, X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

        CalculateRelativePositions();
        // Инициализация состояния
        bte2_state.a1 = bte2_state.b1 = bte2_state.y5 = 0;
        bte2_state.fc1 = bte2_state.fc2 = bte2_state.fc3 = 0;
    }

    void Calculate() override {
        if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
            S8 a = (FInputs[0].Value == TTernary::POS) ? 1 : 0;
            S8 b = (FInputs[1].Value == TTernary::POS) ? 1 : 0;

            bte_2_fn(a, b, &bte2_state);

            FOutputs[0].Value = (bte2_state.y5 == 1) ? TTernary::POS : TTernary::ZERO;
        }
    }

    void Draw(TCanvas* Canvas) override {
        DrawSetunSymbol(Canvas, "BTE-2");
    }

    virtual String GetClassName() const override { return "TBTE2Element"; }
};

// Элемент BTE-3: Y5 = A | B
class TBTE3Element : public TSetunElement {
private:
    bte_3_st_t bte3_state;

public:
    TBTE3Element(int AId, int X, int Y)
        : TSetunElement(AId, "BTE-3", X, Y) {

        FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FOutputs.push_back(TConnectionPoint(this, X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

        CalculateRelativePositions();
        // Инициализация состояния
        bte3_state.a1 = bte3_state.b1 = bte3_state.y5 = 0;
        bte3_state.fc1 = bte3_state.fc2 = bte3_state.fc3 = 0;
    }

    void Calculate() override {
        if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
            S8 a = (FInputs[0].Value == TTernary::POS) ? 1 : 0;
            S8 b = (FInputs[1].Value == TTernary::POS) ? 1 : 0;

            bte_3_fn(a, b, &bte3_state);

            FOutputs[0].Value = (bte3_state.y5 == 1) ? TTernary::POS : TTernary::ZERO;
        }
    }

    void Draw(TCanvas* Canvas) override {
        DrawSetunSymbol(Canvas, "BTE-3");
    }

    virtual String GetClassName() const override { return "TBTE3Element"; }
};

// Элемент TTE-0: Y5 = B * NOT(A), Y7 = A * NOT(B)
class TTTE0Element : public TSetunElement {
private:
    tte_0_st_t tte0_state;

public:
    TTTE0Element(int AId, int X, int Y)
        : TSetunElement(AId, "TTE-0", X, Y) {

        FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FOutputs.push_back(TConnectionPoint(this, X+95, Y+20, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
        FOutputs.push_back(TConnectionPoint(this, X+95, Y+40, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

        CalculateRelativePositions();
        // Инициализация состояния
        tte0_state.a2 = tte0_state.b4 = tte0_state.y5 = tte0_state.y7 = 0;
        tte0_state.fc1 = tte0_state.fc2 = 0;
    }

    void Calculate() override {
        if (FInputs.size() >= 2 && FOutputs.size() >= 2) {
            S8 a = (FInputs[0].Value == TTernary::POS) ? 1 : 0;
            S8 b = (FInputs[1].Value == TTernary::POS) ? 1 : 0;

            tte_0_fn(a, b, &tte0_state);

            FOutputs[0].Value = (tte0_state.y5 == 1) ? TTernary::POS : TTernary::ZERO;
            FOutputs[1].Value = (tte0_state.y7 == 1) ? TTernary::POS : TTernary::ZERO;
        }
    }

    void Draw(TCanvas* Canvas) override {
        DrawSetunSymbol(Canvas, "TTE-0");
    }

    virtual String GetClassName() const override { return "TTTE0Element"; }
};

// Элемент TTE-71: Логическое И (Y8 = A * B)
class TTTE71Element : public TSetunElement {
private:
    tte_71_st_t tte71_state;

public:
    TTTE71Element(int AId, int X, int Y)
        : TSetunElement(AId, "TTE-71", X, Y) {

        FInputs.push_back(TConnectionPoint(this, X-15, Y+20, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FInputs.push_back(TConnectionPoint(this, X-15, Y+40, TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
        FOutputs.push_back(TConnectionPoint(this, X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

        CalculateRelativePositions();
        // Инициализация состояния
        tte71_state.a0 = tte71_state.a1 = tte71_state.b2 = tte71_state.b3 = tte71_state.y8 = 0;
        tte71_state.fc1 = tte71_state.fc2 = 0;
    }

    void Calculate() override {
        if (FInputs.size() >= 2 && FOutputs.size() >= 1) {
            S8 a = (FInputs[0].Value == TTernary::POS) ? 1 : 0;
            S8 b = (FInputs[1].Value == TTernary::POS) ? 1 : 0;

            tte_71_fn(a, b, 0, 0, &tte71_state);

            FOutputs[0].Value = (tte71_state.y8 == 1) ? TTernary::POS : TTernary::ZERO;
        }
    }

    void Draw(TCanvas* Canvas) override {
        DrawSetunSymbol(Canvas, "TTE-71");
    }

    virtual String GetClassName() const override { return "TTTE71Element"; }
};

// Элемент TTE-1: Генератор единицы (Y7 = 1)
class TTTE1Element : public TSetunElement {
private:
    tte_1_st_t tte1_state;

public:
    TTTE1Element(int AId, int X, int Y)
        : TSetunElement(AId, "TTE-1", X, Y) {

        FOutputs.push_back(TConnectionPoint(this, X+95, Y+30, TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));

        CalculateRelativePositions();
        // Инициализация состояния
        tte1_state.y7 = 0;
        tte1_state.fc1 = tte1_state.fc2 = 0;
    }

    void Calculate() override {
        if (FOutputs.size() >= 1) {
            tte_1_fn(&tte1_state);
            FOutputs[0].Value = (tte1_state.y7 == 1) ? TTernary::POS : TTernary::ZERO;
        }
    }

    void Draw(TCanvas* Canvas) override {
        DrawSetunSymbol(Canvas, "TTE-1");
    }

    virtual String GetClassName() const override { return "TTTE1Element"; }
};

// ============================================================================
// ФУНКЦИИ РЕГИСТРАЦИИ DLL
// ============================================================================

extern "C" {

DLL_EXPORT bool __stdcall RegisterLibrary(TLibraryManager* libraryManager) {
    if (!libraryManager) {
        return false;
    }

    try {
        auto emulatorLib = std::make_unique<TComponentLibrary>(
            EMULATOR_SETUN_1958_NAME,
            EMULATOR_SETUN_1958_DESCRIPTION,
            EMULATOR_SETUN_1958_VERSION
        );

        // Регистрируем элементы Сетунь-1958
        emulatorLib->RegisterElement<TBTE1Element>(
            "BTE-1",
            "Базовый троичный элемент 1: Y5 = A * NOT(B)",
            "Базовые элементы",
            80, 60);

        emulatorLib->RegisterElement<TBTE2Element>(
            "BTE-2",
            "Базовый троичный элемент 2: Y5 = A * B",
            "Базовые элементы",
            80, 60);

        emulatorLib->RegisterElement<TBTE3Element>(
            "BTE-3",
            "Базовый троичный элемент 3: Y5 = A | B",
            "Базовые элементы",
            80, 60);

        emulatorLib->RegisterElement<TTTE0Element>(
            "TTE-0",
            "Троичный элемент 0: Y5 = B*NOT(A), Y7 = A*NOT(B)",
            "Троичные элементы",
            80, 60);

        emulatorLib->RegisterElement<TTTE71Element>(
            "TTE-71",
            "Троичный элемент 71: Логическое И (Y8 = A * B)",
            "Логические элементы",
            80, 60);

        emulatorLib->RegisterElement<TTTE1Element>(
            "TTE-1",
            "Троичный элемент 1: Генератор единицы (Y7 = 1)",
            "Генераторы",
            60, 40);

        // Сохраняем указатель для возможности выгрузки
        GEmulatorSetun1958Library = emulatorLib.get();

        // Регистрируем в менеджере
        libraryManager->RegisterLibrary(std::move(emulatorLib));
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
    catch (...) {
        return false;
    }
}

DLL_EXPORT void __stdcall UnregisterLibrary(TLibraryManager* libraryManager) {
    if (libraryManager && GEmulatorSetun1958Library) {
        libraryManager->UnregisterLibrary(GEmulatorSetun1958Library->Name);
        GEmulatorSetun1958Library = nullptr;
    }
}

DLL_EXPORT const char* __stdcall GetLibraryName() {
    return EMULATOR_SETUN_1958_NAME;
}

DLL_EXPORT const char* __stdcall GetLibraryVersion() {
    return EMULATOR_SETUN_1958_VERSION;
}

DLL_EXPORT const char* __stdcall GetLibraryDescription() {
    return EMULATOR_SETUN_1958_DESCRIPTION;
}

} // extern "C"

// Точка входа DLL
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
