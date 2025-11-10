#include "MainForm.h"
#include "CircuitElement.h"
#include "CircuitElements.h"
#include "ComponentLibrary.h"
#include <Vcl.Dialogs.hpp>
#include <Vcl.Graphics.hpp>
#include <math.h>
#include <algorithm>
#include <memory>
#include <System.IOUtils.hpp>
#include <System.IniFiles.hpp>

#pragma package(smart_init)
#pragma resource "*.dfm"

TMainForm *MainForm;

// Временный класс для загрузки соединений
class TTempCircuitElement : public TCircuitElement {
public:
    TTempCircuitElement(int AId) : TCircuitElement(AId, "Temp", 0, 0) {}
    void Calculate() override {
        // Пустая реализация для временного элемента
    }
};

// Реализация TSubCircuit с правильной семантикой перемещения
TSubCircuit::TSubCircuit(int AId, int X, int Y,
                        std::vector<std::unique_ptr<TCircuitElement>>&& Elements,
                        const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& Connections)
    : TCircuitElement(AId, "SubCircuit", X, Y), FAssociatedTab(nullptr) {

    FBounds = TRect(X, Y, X + 120, Y + 80);
    FInternalElements = std::move(Elements);
    FInternalConnections = Connections;

    CreateExternalConnections();
    CalculateRelativePositions();
}

void TSubCircuit::Calculate() {
    for (auto& element : FInternalElements) {
        element->Calculate();
    }

    for (auto& connection : FInternalConnections) {
        if (connection.first && connection.second) {
            connection.second->Value = connection.first->Value;
        }
    }

    UpdateExternalConnections();
}

void TSubCircuit::Draw(TCanvas* Canvas) {
    Canvas->Brush->Color = clWhite;
    Canvas->Pen->Color = clPurple;
    Canvas->Pen->Width = 2;
    Canvas->Rectangle(FBounds.Left, FBounds.Top, FBounds.Right, FBounds.Bottom);
    Canvas->Pen->Width = 1;
    Canvas->Pen->Color = clBlack;

    Canvas->Font->Size = 8;
    Canvas->Font->Color = clPurple;
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 5, "SubCircuit");
    Canvas->TextOut(FBounds.Left + 5, FBounds.Top + 20, "Elems: " + IntToStr(static_cast<int>(FInternalElements.size())));
    Canvas->Font->Color = clBlack;

    DrawConnectionPoints(Canvas);
}

void TSubCircuit::CreateExternalConnections() {
    FInputs.clear();
    FOutputs.clear();

    std::vector<const TConnectionPoint*> externalInputs;
    std::vector<const TConnectionPoint*> externalOutputs;

    for (const auto& element : FInternalElements) {
        for (const auto& input : element->Inputs) {
            bool isInternal = false;
            for (const auto& conn : FInternalConnections) {
                if (conn.second == &input) {
                    isInternal = true;
                    break;
                }
            }
            if (!isInternal) {
                externalInputs.push_back(&input);
            }
        }

        for (const auto& output : element->Outputs) {
            bool isInternal = false;
            for (const auto& conn : FInternalConnections) {
                if (conn.first == &output) {
                    isInternal = true;
                    break;
                }
            }
            if (!isInternal) {
                externalOutputs.push_back(&output);
            }
        }
    }

    int inputSpacing = FBounds.Height() / (externalInputs.size() + 1);
    for (size_t i = 0; i < externalInputs.size(); i++) {
        int y = FBounds.Top + inputSpacing * (i + 1);
        FInputs.push_back(TConnectionPoint(this, FBounds.Left - 15, y,
            TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    }

    int outputSpacing = FBounds.Height() / (externalOutputs.size() + 1);
    for (size_t i = 0; i < externalOutputs.size(); i++) {
        int y = FBounds.Top + outputSpacing * (i + 1);
        FOutputs.push_back(TConnectionPoint(this, FBounds.Right + 15, y,
            TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    }

    if (FInputs.empty()) {
        FInputs.push_back(TConnectionPoint(this, FBounds.Left - 15, FBounds.Top + FBounds.Height()/2 - 10,
            TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    }
    if (FOutputs.empty()) {
        FOutputs.push_back(TConnectionPoint(this, FBounds.Right + 15, FBounds.Top + FBounds.Height()/2 + 10,
            TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    }

    CalculateRelativePositions();
}

void TSubCircuit::UpdateExternalConnections() {
    if (!FInputs.empty() && !FInternalElements.empty() && !FInternalElements[0]->Inputs.empty()) {
        FInternalElements[0]->Inputs[0].Value = FInputs[0].Value;
    }

    if (!FOutputs.empty() && !FInternalElements.empty() && !FInternalElements[0]->Outputs.empty()) {
        FOutputs[0].Value = FInternalElements[0]->Outputs[0].Value;
    }
}

void TSubCircuit::SaveToIni(TIniFile* IniFile, const String& Section) const {
    TCircuitElement::SaveToIni(IniFile, Section);

    IniFile->WriteInteger(Section, "InternalElementCount", static_cast<int>(FInternalElements.size()));

    for (int i = 0; i < FInternalElements.size(); i++) {
        String internalSection = Section + "_Internal_" + IntToStr(i);
        MainForm->SaveElementToIni(FInternalElements[i].get(), IniFile, internalSection);
    }

    IniFile->WriteInteger(Section, "InternalConnectionCount", static_cast<int>(FInternalConnections.size()));

    for (int i = 0; i < FInternalConnections.size(); i++) {
        String connSection = Section + "_InternalConn_" + IntToStr(i);
        auto& conn = FInternalConnections[i];

        if (conn.first && conn.second) {
            MainForm->SaveConnectionPoint(conn.first, IniFile, connSection, "From");
            MainForm->SaveConnectionPoint(conn.second, IniFile, connSection, "To");
        }
    }
}

void TSubCircuit::LoadFromIni(TIniFile* IniFile, const String& Section) {
    TCircuitElement::LoadFromIni(IniFile, Section);

    int internalElementCount = IniFile->ReadInteger(Section, "InternalElementCount", 0);

    for (int i = 0; i < internalElementCount; i++) {
        String internalSection = Section + "_Internal_" + IntToStr(i);
        auto element = MainForm->LoadElementFromIni(IniFile, internalSection);

        if (element) {
            FInternalElements.push_back(std::move(element));
        }
    }

    int internalConnCount = IniFile->ReadInteger(Section, "InternalConnectionCount", 0);

    for (int i = 0; i < internalConnCount; i++) {
        String connSection = Section + "_InternalConn_" + IntToStr(i);

        TConnectionPoint* fromPoint = MainForm->LoadConnectionPoint(IniFile, connSection, "From", this);
        TConnectionPoint* toPoint = MainForm->LoadConnectionPoint(IniFile, connSection, "To", this);

        if (fromPoint && toPoint) {
            TConnectionPoint* realFromPoint = FindConnectionPointInInternalElements(fromPoint);
            TConnectionPoint* realToPoint = FindConnectionPointInInternalElements(toPoint);

            if (realFromPoint && realToPoint) {
                FInternalConnections.push_back(std::make_pair(realFromPoint, realToPoint));
            }

            delete fromPoint;
            delete toPoint;
        }
    }

    CreateExternalConnections();
}

TCircuitElement* TSubCircuit::FindInternalElementById(int Id) {
    for (auto& element : FInternalElements) {
        if (element->Id == Id) {
            return element.get();
        }
    }
    return nullptr;
}

TConnectionPoint* TSubCircuit::FindConnectionPointInInternalElements(const TConnectionPoint* pointTemplate) {
    if (!pointTemplate || !pointTemplate->Owner) return nullptr;

    TCircuitElement* element = FindInternalElementById(pointTemplate->Owner->Id);
    if (!element) return nullptr;

    auto& points = pointTemplate->IsInput ? element->Inputs : element->Outputs;

    for (auto& point : points) {
        if (fabs(point.RelX - pointTemplate->RelX) < 0.001 &&
            fabs(point.RelY - pointTemplate->RelY) < 0.001) {
            return &point;
        }
    }

    return nullptr;
}
// Конструктор MainForm
__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner),
    FSelectedElement(nullptr), FDraggedElement(nullptr), FConnectionStart(nullptr),
    FIsConnecting(false), FIsDragging(false), FIsSelecting(false),
    FNextElementId(1), FDragOffsetX(0), FDragOffsetY(0), FZoomFactor(1.0),
    FScrollOffsetX(0), FScrollOffsetY(0), FSimulationRunning(false), FSimulationStep(0),
    FHotCloseTabIndex(-1), FRectangularConnections(false), FSnapToGrid(true), FIsDrawingWire(false), FWireStartPoint(nullptr) {

    DoubleBuffered = true;
    WorkspacePanel->DoubleBuffered = true;
    miRectangularConnections->Checked = true;
    KeyPreview = true;

    SchemePageControl->OwnerDraw = true;
    SchemePageControl->OnDrawTab = SchemePageControlDrawTab;
    SchemePageControl->OnMouseDown = SchemePageControlMouseDown;
    SchemePageControl->OnMouseMove = SchemePageControlMouseMove;

    SchemePageControl->TabWidth = 120;
    SchemePageControl->TabHeight = 21;

    FSimulationTimer = new TTimer(this);
    FSimulationTimer->Interval = 500;
    FSimulationTimer->OnTimer = &SimulationTimerTimer;
    FSimulationTimer->Enabled = false;
}

TPoint TMainForm::ScreenToLogical(const TPoint& screenPoint) const {
    return TPoint(
        static_cast<int>(screenPoint.X / FZoomFactor),
        static_cast<int>(screenPoint.Y / FZoomFactor)
    );
}

TPoint TMainForm::LogicalToScreen(const TPoint& logicalPoint) const {
    return TPoint(
        static_cast<int>(logicalPoint.X * FZoomFactor),
        static_cast<int>(logicalPoint.Y * FZoomFactor)
    );
}

TRect TMainForm::LogicalToScreen(const TRect& logicalRect) const {
    return TRect(
        LogicalToScreen(TPoint(logicalRect.Left, logicalRect.Top)),
        LogicalToScreen(TPoint(logicalRect.Right, logicalRect.Bottom))
    );
}

TPoint TMainForm::GetVisibleAreaCenter() const {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || !currentTab->ScrollBox) return TPoint(400, 300);

    int logicalLeft = currentTab->ScrollBox->HorzScrollBar->Position / FZoomFactor;
    int logicalTop = currentTab->ScrollBox->VertScrollBar->Position / FZoomFactor;
    int logicalRight = (currentTab->ScrollBox->HorzScrollBar->Position + currentTab->ScrollBox->ClientWidth) / FZoomFactor;
    int logicalBottom = (currentTab->ScrollBox->VertScrollBar->Position + currentTab->ScrollBox->ClientHeight) / FZoomFactor;

    return TPoint(
        (logicalLeft + logicalRight) / 2,
        (logicalTop + logicalBottom) / 2
    );
}

bool TMainForm::FindFreeLocation(int& x, int& y, int width, int height) {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab) return false;

    int originalX = x;
    int originalY = y;
    int gridSize = 20; // Размер сетки
    int step = gridSize * 2; // шаг смещения в логических пикселях

    for (int attempt = 0; attempt < 50; attempt++) {
        // ПРИВЯЗКА К СЕТКЕ ПРИ ПОИСКЕ МЕСТА
        x = ((x + gridSize/2) / gridSize) * gridSize;
        y = ((y + gridSize/2) / gridSize) * gridSize;

        TRect newRect(x, y, x + width, y + height);
        bool collision = false;

        // Проверяем пересечение с существующими элементами
        for (const auto& element : currentTab->Elements) {
            if (newRect.IntersectsWith(element->Bounds)) {
                collision = true;
                break;
            }
        }

        if (!collision) {
            return true; // Нашли свободное место
        }

        // Сдвигаем по спирали от центра с привязкой к сетке
        int ring = (attempt / 4) + 1;
        switch (attempt % 4) {
            case 0: x = originalX + ring * step; break;  // вправо
            case 1: y = originalY + ring * step; break;  // вниз
            case 2: x = originalX - ring * step; break;  // влево
            case 3: y = originalY - ring * step; break;  // вверх
        }
    }

    // Если не нашли свободное место, используем исходную позицию с привязкой
    x = ((originalX + gridSize/2) / gridSize) * gridSize;
    y = ((originalY + gridSize/2) / gridSize) * gridSize;
    return false;
}

TPoint TMainForm::GetBestPlacementPosition(int width, int height) {
    TPoint visibleCenter = GetVisibleAreaCenter();

    // Привязка размеров к сетке (округление до ближайшего кратного сетке)
    int gridSize = 20;
    width = ((width + gridSize - 1) / gridSize) * gridSize;
	height = ((height + gridSize - 1) / gridSize) * gridSize;

    int x = ((visibleCenter.X - width / 2) / gridSize) * gridSize;
    int y = ((visibleCenter.Y - height / 2) / gridSize) * gridSize;

    if (!FindFreeLocation(x, y, width, height)) {
        for (int attempt = 0; attempt < 50; attempt++) {
            int ring = (attempt / 4) + 1;
            switch (attempt % 4) {
                case 0: x = visibleCenter.X + ring * gridSize * 2; break;
                case 1: y = visibleCenter.Y + ring * gridSize * 2; break;
                case 2: x = visibleCenter.X - ring * gridSize * 2; break;
                case 3: y = visibleCenter.Y - ring * gridSize * 2; break;
            }

            x = (x / gridSize) * gridSize;
            y = (y / gridSize) * gridSize;

            if (FindFreeLocation(x, y, width, height)) {
                break;
            }
        }
    }

    return TPoint(x, y);
}

void TMainForm::DeleteSelectedElements() {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || FSelectedElements.empty()) return;

    std::vector<TCircuitElement*> elementsToDelete = FSelectedElements;

    auto it = currentTab->Connections.begin();
    while (it != currentTab->Connections.end()) {
        bool shouldRemove = false;
        for (auto* element : elementsToDelete) {
            if ((it->first->Owner == element) || (it->second->Owner == element)) {
                shouldRemove = true;
                break;
            }
        }
        if (shouldRemove) {
            it = currentTab->Connections.erase(it);
        } else {
            ++it;
        }
    }

    for (auto* elementToDelete : elementsToDelete) {
        auto elemIt = std::find_if(currentTab->Elements.begin(), currentTab->Elements.end(),
            [elementToDelete](const std::unique_ptr<TCircuitElement>& elem) {
                return elem.get() == elementToDelete;
            });

        if (elemIt != currentTab->Elements.end()) {
            currentTab->Elements.erase(elemIt);
        }
    }

    FSelectedElements.clear();
    FSelectedElement = nullptr;

    UpdatePaintBoxSize();
    if (currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
    StatusBar->Panels->Items[0]->Text = "Удалено элементов: " + IntToStr(static_cast<int>(elementsToDelete.size()));
}

void __fastcall TMainForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
    if (Key == VK_DELETE) {
        DeleteSelectedElements();
        Key = 0;
    }
}
void TMainForm::LoadAllLibraries() {
    String currentDir = GetCurrentDir();
    String searchPattern = currentDir + "\\*.dll";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPattern.w_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        StatusBar->Panels->Items[0]->Text = "Не найдены DLL файлы в папке приложения";
        return;
    }

    int loadedCount = 0;
    int totalCount = 0;

    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        String dllFileName = findFileData.cFileName;
        String dllPath = currentDir + "\\" + dllFileName;

        totalCount++;

        if (LoadLibraryFromDLL(dllPath)) {
            loadedCount++;
            StatusBar->Panels->Items[0]->Text = "Загружена библиотека: " + dllFileName;
        }

    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    String resultMsg = "Загружено библиотек: " + IntToStr(loadedCount) + " из " + IntToStr(totalCount);
    StatusBar->Panels->Items[0]->Text = resultMsg;

    UpdateLibrarySelector();
}

void TMainForm::UnloadAllLibraries() {
    for (auto it = FLoadedLibraries.rbegin(); it != FLoadedLibraries.rend(); ++it) {
        TLoadedLibrary& lib = *it;

        if (lib.UnregisterFunc) {
            lib.UnregisterFunc(FLibraryManager.get());
        }

        if (lib.Handle) {
            FreeLibrary(lib.Handle);
        }
    }

    FLoadedLibraries.clear();
}

bool TMainForm::LoadLibraryFromDLL(const String& DllPath) {
    String fileName = ExtractFileName(DllPath).LowerCase();
    if (fileName == "vcl.dll" || fileName == "rtl.dll" ||
        fileName.Pos("borland") > 0 || fileName.Pos("bpl") > 0) {
        return false;
    }

    HINSTANCE libraryHandle = LoadLibrary(DllPath.w_str());

    if (!libraryHandle) {
        return false;
    }

    TRegisterLibraryFunction registerFunc = FindRegisterFunction(libraryHandle);

    if (!registerFunc) {
        FreeLibrary(libraryHandle);
        return false;
    }

    TUnregisterLibraryFunction unregisterFunc = FindUnregisterFunction(libraryHandle);

    try {
        bool success = registerFunc(FLibraryManager.get());

        if (!success) {
            FreeLibrary(libraryHandle);
            return false;
        }

        TLoadedLibrary loadedLib;
        loadedLib.Handle = libraryHandle;
        loadedLib.FileName = ExtractFileName(DllPath);
        loadedLib.UnregisterFunc = unregisterFunc;

        typedef const char* (__stdcall *TGetLibraryNameFunction)();
        TGetLibraryNameFunction getNameFunc = reinterpret_cast<TGetLibraryNameFunction>(
            GetProcAddress(libraryHandle, "GetLibraryName"));

        if (getNameFunc) {
            loadedLib.LibraryName = String(getNameFunc());
        } else {
            loadedLib.LibraryName = "Библиотека из " + loadedLib.FileName;
        }

        FLoadedLibraries.push_back(loadedLib);
        return true;
    }
    catch (...) {
        FreeLibrary(libraryHandle);
        return false;
    }
}

TRegisterLibraryFunction TMainForm::FindRegisterFunction(HINSTANCE LibraryHandle) {
    const char* functionNames[] = {
        "RegisterLibrary",
        "RegisterStandardLibrary",
        "_RegisterLibrary@4",
        "_RegisterStandardLibrary@4",
        "RegisterLibrary@4",
        "RegisterStandardLibrary@4"
    };

    for (const char* funcName : functionNames) {
        TRegisterLibraryFunction func = reinterpret_cast<TRegisterLibraryFunction>(
            GetProcAddress(LibraryHandle, funcName));
        if (func) return func;
    }

    return nullptr;
}

TUnregisterLibraryFunction TMainForm::FindUnregisterFunction(HINSTANCE LibraryHandle) {
    const char* functionNames[] = {
        "UnregisterLibrary",
        "UnregisterStandardLibrary",
        "_UnregisterLibrary@4",
        "_UnregisterStandardLibrary@4",
        "UnregisterLibrary@4",
        "UnregisterStandardLibrary@4"
    };

    for (const char* funcName : functionNames) {
        TUnregisterLibraryFunction func = reinterpret_cast<TUnregisterLibraryFunction>(
            GetProcAddress(LibraryHandle, funcName));
        if (func) return func;
    }

    return nullptr;
}

void __fastcall TMainForm::FormCreate(TObject *Sender) {
    FLibraryManager = std::make_unique<TLibraryManager>();
    CreateBasicLibrary();
    LoadAllLibraries();

    CreateCompleteLibrary();

    TTabSheet* mainTab = CreateNewTab("Основная схема");
    SchemePageControl->ActivePage = mainTab;

    btnConnectionMode->AllowAllUp = true;
    btnConnectionMode->GroupIndex = 1;

    UpdateLibrarySelector();

    StatusBar->Panels->Items[0]->Text = "Готов к работе. Выберите элемент из библиотеки или режим соединения.";
    Caption = "Setun IDE - Троичная логика по книге 1965 года";
}

void TMainForm::CreateBasicLibrary() {
    FBasicLibrary = std::make_unique<TComponentLibrary>("Basic", "Базовая библиотека элементов", "1.0");

    FBasicLibrary->RegisterElement<TMagneticAmplifier>("Магнитный усилитель", "Простой магнитный усилитель", "Усилители");
    FBasicLibrary->RegisterElement<TTernaryElement>("Троичный элемент", "Базовый троичный элемент", "Логика");
    FBasicLibrary->RegisterElement<TShiftRegister>("Сдвигающий регистр", "4-битный сдвигающий регистр", "Память");
    FBasicLibrary->RegisterElement<TTernaryTrigger>("Троичный триггер", "Троичный триггер", "Память");
    FBasicLibrary->RegisterElement<THalfAdder>("Полусумматор", "Троичный полусумматор", "Арифметика");
    FBasicLibrary->RegisterElement<TTernaryAdder>("Троичный сумматор", "Троичный сумматор", "Арифметика");
    FBasicLibrary->RegisterElement<TDecoder>("Дешифратор", "Дешифратор троичного кода", "Логика");
    FBasicLibrary->RegisterElement<TCounter>("Счетчик", "Троичный счетчик", "Память");
    FBasicLibrary->RegisterElement<TLogicAnd>("Логическое И", "Логический элемент И", "Логика");
    FBasicLibrary->RegisterElement<TLogicOr>("Логическое ИЛИ", "Логический элемент ИЛИ", "Логика");
    FBasicLibrary->RegisterElement<TLogicInhibit>("Запрет", "Схема запрета", "Логика");
    FBasicLibrary->RegisterElement<TGenerator>("Генератор", "Генератор единиц", "Источники");

    FLibraryManager->RegisterLibrary(std::move(FBasicLibrary));
}

void __fastcall TMainForm::FormDestroy(TObject *Sender) {
    FSimulationTimer->Enabled = false;

    FElements.clear();
    FConnections.clear();
    FSelectedElements.clear();

    FLibraryManager.reset();
    UnloadAllLibraries();
}

std::unique_ptr<TCircuitElement> TMainForm::CreateElement(const String& LibraryName, const String& ElementName, int X, int Y) {
    try {
        if (!FLibraryManager) {
            throw Exception("Менеджер библиотеков не инициализирован");
        }

        auto element = FLibraryManager->CreateElement(LibraryName, ElementName, FNextElementId, X, Y);
        if (element) {
            element->CalculateRelativePositions();
            FNextElementId++;
        } else {
            throw Exception("Библиотека вернула nullptr для элемента '" + ElementName + "' из библиотеки '" + LibraryName + "'");
        }
        return element;
    }
    catch (Exception &e) {
        String errorMsg = "Ошибка создания элемента '" + ElementName + "' из библиотеки '" + LibraryName + "': " + e.Message;
        ShowMessage(errorMsg);
        StatusBar->Panels->Items[0]->Text = errorMsg;
        return nullptr;
    }
}

std::unique_ptr<TCircuitElement> TMainForm::CreateElementFromCurrent(const String& ElementName, int X, int Y) {
    try {
        if (!FLibraryManager) {
            throw Exception("Менеджер библиотеков не инициализирован");
        }

        if (!FLibraryManager->GetCurrentLibrary()) {
            throw Exception("Текущая библиотека не установлена");
        }

        auto element = FLibraryManager->CreateElementFromCurrent(ElementName, FNextElementId, X, Y);
        if (element) {
            element->CalculateRelativePositions();
            FNextElementId++;
        } else {
            throw Exception("Библиотека вернула nullptr для элемента: " + ElementName);
        }
        return element;
    }
    catch (Exception &e) {
        String errorMsg = "Ошибка создания элемента '" + ElementName + "': " + e.Message;
        ShowMessage(errorMsg);
        StatusBar->Panels->Items[0]->Text = errorMsg;
        return nullptr;
    }
}

void TMainForm::UpdateLibrarySelector() {
    cmbLibrarySelector->Items->Clear();
    if (!FLibraryManager) return;

    auto libraryNames = FLibraryManager->GetLibraryNames();
    for (const auto& name : libraryNames) {
        cmbLibrarySelector->Items->Add(name);
    }

    if (!libraryNames.empty()) {
        cmbLibrarySelector->ItemIndex = 0;
        LoadCurrentLibrary();
    }
}

void TMainForm::LoadCurrentLibrary() {
    ElementLibrary->Items->Clear();

    if (!FLibraryManager || !FLibraryManager->GetCurrentLibrary()) return;

    auto currentLib = FLibraryManager->GetCurrentLibrary();
    auto elements = currentLib->GetElementNames();
    for (const auto& element : elements) {
        ElementLibrary->Items->Add(element);
    }

    LibraryLabel->Caption = "Библиотека: " + currentLib->Name;
    StatusBar->Panels->Items[0]->Text = "Загружена библиотека: " + currentLib->Name;
}

void __fastcall TMainForm::cmbLibrarySelectorChange(TObject *Sender) {
    if (cmbLibrarySelector->ItemIndex >= 0 && FLibraryManager) {
        String libraryName = cmbLibrarySelector->Items->Strings[cmbLibrarySelector->ItemIndex];
        try {
            FLibraryManager->SetCurrentLibrary(libraryName);
            LoadCurrentLibrary();
        }
        catch (Exception &e) {
            ShowMessage("Ошибка переключения библиотеки: " + e.Message);
        }
    }
}

void TMainForm::CreateCompleteLibrary() {
    UpdateLibrarySelector();
}

void __fastcall TMainForm::ElementLibraryDblClick(TObject *Sender) {
    if (ElementLibrary->ItemIndex < 0) return;

    String elementName = ElementLibrary->Items->Strings[ElementLibrary->ItemIndex];

    // Проверяем режим только для чтения
    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->IsReadOnly) {
        StatusBar->Panels->Items[0]->Text = "Режим просмотра: добавление элементов запрещено";
        return;
    }

    // Создаем элемент временно в (0,0) чтобы получить его размеры
    auto newElement = CreateElementFromCurrent(elementName, 0, 0);
    if (!newElement) {
        StatusBar->Panels->Items[0]->Text = "Ошибка создания элемента: " + elementName;
        return;
    }

    int width = newElement->Bounds.Width();
    int height = newElement->Bounds.Height();

    // Получаем оптимальную позицию для размещения
    TPoint bestPos = GetBestPlacementPosition(width, height);

    // ПРИВЯЗКА К СЕТКЕ: убедимся, что позиция привязана к сетке
    bestPos = SnapToGridPoint(bestPos);

    // Устанавливаем новые границы
    newElement->SetBounds(TRect(bestPos.X, bestPos.Y, bestPos.X + width, bestPos.Y + height));
	newElement->CalculateRelativePositions();


	newElement->SetBounds(TRect(bestPos.X, bestPos.Y, bestPos.X + width, bestPos.Y + height));
	newElement->CalculateRelativePositions(); // Это привяжет точки к сетке

	// принудительный пересчет еще раз после установки границ:
	newElement->CalculateRelativePositions();

    if (currentTab) {
        currentTab->Elements.push_back(std::move(newElement));
        UpdatePaintBoxSize();
        if (currentTab->PaintBox) {
            currentTab->PaintBox->Repaint();
        }
        StatusBar->Panels->Items[0]->Text = "Добавлен: " + elementName;
    }
}


// Новая реализация сериализации для сохранения схемы
void TMainForm::SaveSchemeToFile(const String& FileName, TTabData* TabData) {
    if (!TabData) {
        TabData = GetCurrentTabData();
        if (!TabData) return;
    }

    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    // Сохраняем основную информацию
    iniFile->WriteInteger("Scheme", "ElementCount", static_cast<int>(TabData->Elements.size()));
    iniFile->WriteInteger("Scheme", "ConnectionCount", static_cast<int>(TabData->Connections.size()));
    iniFile->WriteInteger("Scheme", "NextElementId", TabData->NextElementId);
    iniFile->WriteString("Scheme", "Version", "3.0");

    // Сохраняем элементы
    for (int i = 0; i < TabData->Elements.size(); i++) {
        String section = "Element_" + IntToStr(i);
        SaveElementToIni(TabData->Elements[i].get(), iniFile.get(), section);
    }

    // Сохраняем соединения
    for (int i = 0; i < TabData->Connections.size(); i++) {
        String section = "Connection_" + IntToStr(i);
        auto& connection = TabData->Connections[i];

        if (connection.first && connection.second) {
            SaveConnectionPoint(connection.first, iniFile.get(), section, "From");
            SaveConnectionPoint(connection.second, iniFile.get(), section, "To");
        }
    }

    iniFile->UpdateFile();
}

// Новая реализация загрузки схемы
void TMainForm::LoadSchemeFromFile(const String& FileName, TTabData* TabData) {
    if (!TabData) {
        TabData = GetCurrentTabData();
        if (!TabData) return;
    }

    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    String version = iniFile->ReadString("Scheme", "Version", "1.0");
    if (version == "1.0") {
        ShowMessage("Формат файла устарел. Используйте новую версию для сохранения схем.");
        return;
    }

    btnClearWorkspaceClick(nullptr);

    int elementCount = iniFile->ReadInteger("Scheme", "ElementCount", 0);
    int connectionCount = iniFile->ReadInteger("Scheme", "ConnectionCount", 0);
    TabData->NextElementId = iniFile->ReadInteger("Scheme", "NextElementId", 1);

    // Загружаем элементы
    std::map<int, TCircuitElement*> idToElementMap;

    for (int i = 0; i < elementCount; i++) {
        String section = "Element_" + IntToStr(i);
        auto element = LoadElementFromIni(iniFile.get(), section);

        if (element) {
            idToElementMap[element->Id] = element.get();
            TabData->Elements.push_back(std::move(element));
        }
    }

    // Загружаем соединения
    for (int i = 0; i < connectionCount; i++) {
        String section = "Connection_" + IntToStr(i);

        TConnectionPoint* fromPoint = LoadConnectionPoint(iniFile.get(), section, "From", nullptr);
        TConnectionPoint* toPoint = LoadConnectionPoint(iniFile.get(), section, "To", nullptr);

        if (fromPoint && toPoint) {
            // Находим реальные точки соединения в загруженных элементах
            TCircuitElement* fromElement = idToElementMap[fromPoint->Owner->Id];
            TCircuitElement* toElement = idToElementMap[toPoint->Owner->Id];

            TConnectionPoint* realFromPoint = FindConnectionPointInElement(fromElement, fromPoint);
            TConnectionPoint* realToPoint = FindConnectionPointInElement(toElement, toPoint);

            if (realFromPoint && realToPoint) {
                TabData->Connections.push_back(std::make_pair(realFromPoint, realToPoint));
            }

            delete fromPoint;
            delete toPoint;
        }
    }

    UpdatePaintBoxSize();
    CenterCircuit();
    if (TabData->PaintBox) {
        TabData->PaintBox->Repaint();
    }

    StatusBar->Panels->Items[0]->Text = "Схема загружена: " + FileName +
                                       " (элементов: " + IntToStr(elementCount) + ")";
}

// Методы для сериализации элементов
void TMainForm::SaveElementToIni(TCircuitElement* Element, TIniFile* IniFile, const String& Section) {
    if (Element) {
        Element->SaveToIni(IniFile, Section);
    }
}

std::unique_ptr<TCircuitElement> TMainForm::LoadElementFromIni(TIniFile* IniFile, const String& Section) {
    String className = IniFile->ReadString(Section, "ClassName", "");
    int id = IniFile->ReadInteger(Section, "Id", 0);
    int x = IniFile->ReadInteger(Section, "X", 0);
    int y = IniFile->ReadInteger(Section, "Y", 0);

    auto element = CreateElementByClassName(className, id, x, y);
    if (element) {
        element->LoadFromIni(IniFile, Section);
    }

    return element;
}

std::unique_ptr<TCircuitElement> TMainForm::CreateElementByClassName(const String& ClassName, int Id, int X, int Y) {
    if (ClassName == "TMagneticAmplifier") {
        return std::make_unique<TMagneticAmplifier>(Id, X, Y, false);
    } else if (ClassName == "TTernaryElement") {
        return std::make_unique<TTernaryElement>(Id, X, Y);
    } else if (ClassName == "TShiftRegister") {
        return std::make_unique<TShiftRegister>(Id, X, Y, 4);
    } else if (ClassName == "TTernaryTrigger") {
        return std::make_unique<TTernaryTrigger>(Id, X, Y);
    } else if (ClassName == "THalfAdder") {
        return std::make_unique<THalfAdder>(Id, X, Y);
    } else if (ClassName == "TTernaryAdder") {
        return std::make_unique<TTernaryAdder>(Id, X, Y);
    } else if (ClassName == "TDecoder") {
        return std::make_unique<TDecoder>(Id, X, Y, 2);
    } else if (ClassName == "TCounter") {
        return std::make_unique<TCounter>(Id, X, Y, 3);
    } else if (ClassName == "TDistributor") {
        return std::make_unique<TDistributor>(Id, X, Y, 8);
    } else if (ClassName == "TSwitch") {
        return std::make_unique<TSwitch>(Id, X, Y, 3);
    } else if (ClassName == "TLogicAnd") {
        return std::make_unique<TLogicAnd>(Id, X, Y);
    } else if (ClassName == "TLogicOr") {
        return std::make_unique<TLogicOr>(Id, X, Y);
    } else if (ClassName == "TLogicInhibit") {
        return std::make_unique<TLogicInhibit>(Id, X, Y);
    } else if (ClassName == "TGenerator") {
        return std::make_unique<TGenerator>(Id, X, Y);
    } else if (ClassName == "TSubCircuit") {
        return std::make_unique<TSubCircuit>(Id, X, Y,
            std::vector<std::unique_ptr<TCircuitElement>>(),
            std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>());
    }

    return nullptr;
}

void TMainForm::SaveConnectionPoint(const TConnectionPoint* Point, TIniFile* IniFile,
                                   const String& Section, const String& Prefix) {
    if (!Point || !Point->Owner) return;

    IniFile->WriteInteger(Section, Prefix + "ElementId", Point->Owner->Id);
    IniFile->WriteFloat(Section, Prefix + "RelX", Point->RelX);
    IniFile->WriteFloat(Section, Prefix + "RelY", Point->RelY);
    IniFile->WriteBool(Section, Prefix + "IsInput", Point->IsInput);
}

TConnectionPoint* TMainForm::LoadConnectionPoint(TIniFile* IniFile, const String& Section,
                                                const String& Prefix, TCircuitElement* Owner) {
    int elementId = IniFile->ReadInteger(Section, Prefix + "ElementId", -1);
    if (elementId == -1) return nullptr;

    // Создаем временный элемент-заглушку для точки соединения
    auto tempElement = std::make_unique<TTempCircuitElement>(elementId);

    auto point = new TConnectionPoint(
        tempElement.get(),
        0, 0,
        TTernary::ZERO,
        IniFile->ReadBool(Section, Prefix + "IsInput", true),
        TLineStyle::POSITIVE_CONTROL
    );

    point->RelX = IniFile->ReadFloat(Section, Prefix + "RelX", 0);
    point->RelY = IniFile->ReadFloat(Section, Prefix + "RelY", 0);

    // Сохраняем временный элемент, чтобы избежать его уничтожения
    static std::vector<std::unique_ptr<TCircuitElement>> tempElements;
    tempElements.push_back(std::move(tempElement));

    return point;
}

TConnectionPoint* TMainForm::FindConnectionPointInElement(TCircuitElement* element, const TConnectionPoint* pointTemplate) {
    if (!element) return nullptr;

    auto& points = pointTemplate->IsInput ? element->Inputs : element->Outputs;

    for (auto& point : points) {
        if (fabs(point.RelX - pointTemplate->RelX) < 0.001 &&
            fabs(point.RelY - pointTemplate->RelY) < 0.001) {
            return &point;
        }
    }

    return nullptr;
}

void __fastcall TMainForm::miDeleteElementClick(TObject *Sender) {
    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->IsReadOnly) {
        StatusBar->Panels->Items[0]->Text = "Режим просмотра: удаление элементов запрещено";
        return;
    }

    DeleteSelectedElements();
}

void __fastcall TMainForm::btnClearWorkspaceClick(TObject *Sender) {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab) return;

    if (Application->MessageBox(L"Очистить всю рабочую область?", L"Подтверждение",
                               MB_YESNO | MB_ICONQUESTION) == ID_YES) {
        currentTab->Elements.clear();
        currentTab->Connections.clear();
        FSelectedElements.clear();
        FSelectedElement = nullptr;
        currentTab->NextElementId = 1;

        UpdatePaintBoxSize();

        // Сбрасываем скролл в начало после очистки
        if (currentTab->ScrollBox) {
            currentTab->ScrollBox->HorzScrollBar->Position = 0;
            currentTab->ScrollBox->VertScrollBar->Position = 0;
        }

        if (currentTab->PaintBox) {
            currentTab->PaintBox->Repaint();
        }
        StatusBar->Panels->Items[0]->Text = "Рабочая область очищена.";
    }
}

void __fastcall TMainForm::CircuitImagePaint(TObject *Sender) {
    DrawCircuit();
}

void TMainForm::DrawCircuit() {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || !currentTab->PaintBox) return;

    OptimizedDrawCircuit(currentTab->PaintBox->Canvas, currentTab);
}

// ИСПРАВЛЕННАЯ ВЕРСИЯ - добавлен второй параметр
void TMainForm::OptimizedDrawCircuit(TCanvas* Canvas, TTabData* TabData) {
    if (!TabData) return;

    // Используем статический буфер для уменьшения создания/удаления объектов
    static std::unique_ptr<TBitmap> buffer;
    static int lastWidth = 0, lastHeight = 0;

    if (!buffer || lastWidth != TabData->PaintBox->Width || lastHeight != TabData->PaintBox->Height) {
        buffer = std::make_unique<TBitmap>();
        buffer->Width = TabData->PaintBox->Width;
        buffer->Height = TabData->PaintBox->Height;
        lastWidth = TabData->PaintBox->Width;
        lastHeight = TabData->PaintBox->Height;
    }

    TCanvas* canvas = buffer->Canvas;

    // Очистка фона
    canvas->Brush->Color = clWhite;
    canvas->FillRect(TabData->PaintBox->ClientRect);

    // Статическая переменная для отслеживания изменения масштаба
    canvas->Pen->Color = clSilver;
    canvas->Pen->Style = psDot;
    int gridSize = static_cast<int>(20 * FZoomFactor);
    if (gridSize > 2) {
        for (int x = 0; x < TabData->PaintBox->Width; x += gridSize) {
            canvas->MoveTo(x, 0);
            canvas->LineTo(x, TabData->PaintBox->Height);
        }
        for (int y = 0; y < TabData->PaintBox->Height; y += gridSize) {
            canvas->MoveTo(0, y);
            canvas->LineTo(TabData->PaintBox->Width, y);
        }
    }
    canvas->Pen->Style = psSolid;

    // Соединения
    canvas->Pen->Width = static_cast<int>(2 * FZoomFactor);
// В блоке отрисовки соединений ДОБАВЬТЕ привязку координат:
for (auto& connection : TabData->Connections) {
    TConnectionPoint* start = connection.first;
    TConnectionPoint* end = connection.second;

    TColor connectionColor = TernaryToColor(start->Value);

    // ПРИВЯЗКА КООРДИНАТ ТОЧЕК К СЕТКЕ
    TPoint logicalStart = TPoint(start->X, start->Y);
	TPoint logicalEnd = TPoint(end->X, end->Y);

    TPoint screenStart = LogicalToScreen(logicalStart);
    TPoint screenEnd = LogicalToScreen(logicalEnd);

    // Учитываем смещение скролла
    if (TabData->ScrollBox) {
        screenStart.Offset(-TabData->ScrollBox->HorzScrollBar->Position, -TabData->ScrollBox->VertScrollBar->Position);
        screenEnd.Offset(-TabData->ScrollBox->HorzScrollBar->Position, -TabData->ScrollBox->VertScrollBar->Position);
    }

    if (FRectangularConnections) {
        // Прямоугольное соединение
        DrawRectangularConnection(canvas, screenStart, screenEnd, connectionColor);
    } else {
        // Прямое соединение
        canvas->Pen->Color = connectionColor;
        canvas->MoveTo(screenStart.X, screenStart.Y);
        canvas->LineTo(screenEnd.X, screenEnd.Y);

        // Стрелка только для достаточно длинных линий
        int dx = screenEnd.X - screenStart.X;
        int dy = screenEnd.Y - screenStart.Y;
        double length = sqrt(dx*dx + dy*dy);
        if (length > 15) {
            double unitX = dx / length;
            double unitY = dy / length;

            int arrowSize = static_cast<int>(6 * FZoomFactor);
            int arrowX = screenEnd.X - static_cast<int>(unitX * arrowSize);
            int arrowY = screenEnd.Y - static_cast<int>(unitY * arrowSize);

            canvas->MoveTo(arrowX - static_cast<int>(unitY * arrowSize/2), arrowY + static_cast<int>(unitX * arrowSize/2));
            canvas->LineTo(screenEnd.X, screenEnd.Y);
            canvas->LineTo(arrowX + static_cast<int>(unitY * arrowSize/2), arrowY - static_cast<int>(unitX * arrowSize/2));
        }
    }
}
    canvas->Pen->Width = 1;

    // Рисование текущего провода - ИСПРАВЛЕННАЯ СЕКЦИЯ
    if (FIsDrawingWire && FCurrentWirePoints.size() > 0) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Style = psDash;
        canvas->Pen->Width = 2;

        // Преобразуем логические координаты в экранные
        std::vector<TPoint> screenPoints;
        for (const auto& point : FCurrentWirePoints) {
            TPoint screenPoint = LogicalToScreen(point);
            if (TabData->ScrollBox) {
                screenPoint.Offset(-TabData->ScrollBox->HorzScrollBar->Position,
                                  -TabData->ScrollBox->VertScrollBar->Position);
            }
            screenPoints.push_back(screenPoint);
        }

        // Рисуем ломаную линию
        if (screenPoints.size() > 0) {
            canvas->MoveTo(screenPoints[0].X, screenPoints[0].Y);
            for (size_t i = 1; i < screenPoints.size(); i++) {
                canvas->LineTo(screenPoints[i].X, screenPoints[i].Y);
            }

            // Рисуем текущую позицию мыши
            TPoint mousePos = TabData->PaintBox->ScreenToClient(Mouse->CursorPos);
            canvas->LineTo(mousePos.X, mousePos.Y);
        }

        canvas->Pen->Style = psSolid;
        canvas->Pen->Width = 1;
    }


    // Элементы - рисуем в логических координатах с учетом масштаба и смещения
    for (auto& element : TabData->Elements) {
        // Сохраняем оригинальные bounds
        TRect originalBounds = element->Bounds;

        // Создаем временные bounds с учетом масштаба и смещения
        TRect screenBounds = LogicalToScreen(originalBounds);
        if (TabData->ScrollBox) {
            screenBounds.Offset(-TabData->ScrollBox->HorzScrollBar->Position, -TabData->ScrollBox->VertScrollBar->Position);
        }

        // Временно устанавливаем экранные bounds для отрисовки
        element->SetBounds(screenBounds);

        // Рисуем элемент
        element->Draw(canvas);

        // Восстанавливаем оригинальные bounds
        element->SetBounds(originalBounds);
    }

    // Выделенные элементы
    for (auto selectedElement : FSelectedElements) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Width = 2;
        canvas->Pen->Style = psDash;
        canvas->Brush->Style = bsClear;
        TRect screenBounds = LogicalToScreen(selectedElement->Bounds);
        if (TabData->ScrollBox) {
            screenBounds.Offset(-TabData->ScrollBox->HorzScrollBar->Position, -TabData->ScrollBox->VertScrollBar->Position);
        }
        canvas->Rectangle(screenBounds);
        canvas->Pen->Style = psSolid;
        canvas->Pen->Width = 1;
        canvas->Brush->Style = bsSolid;
    }

    // Прямоугольник выделения
    if (FIsSelecting) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Style = psDash;
        canvas->Brush->Style = bsClear;
        canvas->Rectangle(FSelectionRect);
        canvas->Pen->Style = psSolid;
        canvas->Brush->Style = bsSolid;
    }

    // Соединение в процессе
    if (FIsConnecting && FConnectionStart) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Style = psDash;
        TPoint screenStart = LogicalToScreen(TPoint(FConnectionStart->X, FConnectionStart->Y));
        if (TabData->ScrollBox) {
            screenStart.Offset(-TabData->ScrollBox->HorzScrollBar->Position, -TabData->ScrollBox->VertScrollBar->Position);
        }
        canvas->MoveTo(screenStart.X, screenStart.Y);
        TPoint mousePos = TabData->PaintBox->ScreenToClient(Mouse->CursorPos);
        canvas->LineTo(mousePos.X, mousePos.Y);
        canvas->Pen->Style = psSolid;
    }

    // Единоразовая отрисовка буфера на экран
    Canvas->Draw(0, 0, buffer.get());
}

TColor TMainForm::TernaryToColor(TTernary Value) {
    switch (Value) {
        case TTernary::NEG: return clRed;
        case TTernary::ZERO: return clGray;
        case TTernary::POS: return clGreen;
        default: return clBlack;
    }
}
// ИСПРАВЛЕННАЯ СИГНАТУРА - добавлен const для MousePos
void __fastcall TMainForm::WorkspaceMouseWheel(TObject *Sender, TShiftState Shift,
    int WheelDelta, const TPoint &MousePos, bool &Handled) {

    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || !currentTab->ScrollBox) return;

    if (Shift.Contains(ssCtrl)) {
        Handled = true;
        if (WheelDelta > 0) {
            btnZoomInClick(Sender);
        } else {
            btnZoomOutClick(Sender);
        }
    } else {
        // Прокрутка без Ctrl - перемещение по workspace
        Handled = true;
        int scrollAmount = WheelDelta > 0 ? -30 : 30;

        currentTab->ScrollBox->VertScrollBar->Position = currentTab->ScrollBox->VertScrollBar->Position + scrollAmount;
        currentTab->ScrollBox->HorzScrollBar->Position = currentTab->ScrollBox->HorzScrollBar->Position + scrollAmount;

        if (currentTab->PaintBox) {
            currentTab->PaintBox->Repaint();
        }
    }
}

void __fastcall TMainForm::CircuitImageMouseDown(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y) {

    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || !currentTab->ScrollBox) return;

    // Проверяем режим только для чтения
    if (currentTab->IsReadOnly) {
        StatusBar->Panels->Items[0]->Text = "Режим просмотра: редактирование запрещено";
        return;
    }

    // Преобразуем экранные координаты в логические
    TPoint logicalPos = ScreenToLogical(TPoint(
        X + currentTab->ScrollBox->HorzScrollBar->Position,
        Y + currentTab->ScrollBox->VertScrollBar->Position
    ));

    if (Button == mbLeft) {
        // ПЕРВОЕ: проверяем клик на точку соединения ВНЕ зависимости от текущего режима
        for (auto& element : currentTab->Elements) {
            TConnectionPoint* conn = element->GetConnectionAt(logicalPos.X, logicalPos.Y);
            if (conn) {
                if (!FIsConnecting) {
                    // Начало нового соединения
                    if (!conn->IsInput) {
                        // Клик на выходной точке - начинаем соединение
                        FConnectionStart = conn;
                        FIsConnecting = true;
                        btnConnectionMode->Down = true;
                        StatusBar->Panels->Items[0]->Text = "Режим соединения. Выберите входную точку.";
                    } else {
                        StatusBar->Panels->Items[0]->Text = "Ошибка: первая точка должна быть выходом (синяя)";
                    }
                    if (currentTab->PaintBox) {
                        currentTab->PaintBox->Repaint();
                    }
                    return;
                } else {
                    // Завершение существующего соединения
                    if (conn->IsInput && FConnectionStart && FConnectionStart->Owner != conn->Owner) {
                        bool connectionExists = false;
                        for (auto& existingConn : currentTab->Connections) {
                            if (existingConn.first == FConnectionStart && existingConn.second == conn) {
                                connectionExists = true;
                                break;
                            }
                        }

                        if (!connectionExists) {
                            currentTab->Connections.push_back(std::make_pair(FConnectionStart, conn));
                            StatusBar->Panels->Items[0]->Text = "Соединение создано.";
                        } else {
                            StatusBar->Panels->Items[0]->Text = "Соединение уже существует.";
                        }
                    } else {
                        StatusBar->Panels->Items[0]->Text = "Ошибка: вторая точка должна быть входом (зеленая) и принадлежать другому элементу";
                    }
                    FIsConnecting = false;
                    FConnectionStart = nullptr;
                    btnConnectionMode->Down = false;
                    if (currentTab->PaintBox) {
                        currentTab->PaintBox->Repaint();
                    }
                }
                return;
            }
        }

        // Если кликнули на пустом месте в режиме соединения - выходим из режима
        if (FIsConnecting) {
            FIsConnecting = false;
            FConnectionStart = nullptr;
            btnConnectionMode->Down = false;
            StatusBar->Panels->Items[0]->Text = "Режим соединения отменен.";
            if (currentTab->PaintBox) {
                currentTab->PaintBox->Repaint();
            }
            return;
        }

        // Стандартная обработка выделения и перемещения элементов
        FSelectedElement = nullptr;
        FDraggedElement = nullptr;

        if (!Shift.Contains(ssCtrl)) {
            FSelectedElements.clear();
        }

        bool elementFound = false;

        for (auto& element : currentTab->Elements) {
            TRect bounds = element->Bounds;
            TRect expandedBounds = TRect(
                bounds.Left - 5, bounds.Top - 5,
                bounds.Right + 5, bounds.Bottom + 5
            );

            if (logicalPos.X >= expandedBounds.Left && logicalPos.X <= expandedBounds.Right &&
                logicalPos.Y >= expandedBounds.Top && logicalPos.Y <= expandedBounds.Bottom) {

                FSelectedElement = element.get();
                elementFound = true;

                auto it = std::find(FSelectedElements.begin(), FSelectedElements.end(), element.get());
                if (it == FSelectedElements.end()) {
                    FSelectedElements.push_back(element.get());
                }

                FDraggedElement = element.get();
                FIsDragging = true;

                FDragOffsetX = logicalPos.X - bounds.Left;
                FDragOffsetY = logicalPos.Y - bounds.Top;

                StatusBar->Panels->Items[0]->Text = "Выбран элемент: " + element->Name;
                break;
            }
        }

        if (!elementFound) {
            FIsSelecting = true;
            FSelectionRect = TRect(X, Y, X, Y);
            if (!Shift.Contains(ssCtrl)) {
                FSelectedElements.clear();
            }
            StatusBar->Panels->Items[0]->Text = "Режим выделения: рисуйте прямоугольник";
        } else {
            StatusBar->Panels->Items[0]->Text = "Выбрано элементов: " + IntToStr(static_cast<int>(FSelectedElements.size()));
        }

        if (currentTab->PaintBox) {
            currentTab->PaintBox->Repaint();
        }
    } else if (Button == mbRight) {
        FSelectedElement = nullptr;
        for (auto& element : currentTab->Elements) {
            TRect bounds = element->Bounds;
            if (logicalPos.X >= bounds.Left && logicalPos.X <= bounds.Right &&
                logicalPos.Y >= bounds.Top && logicalPos.Y <= bounds.Bottom) {
                FSelectedElement = element.get();

                TPoint popupPos = currentTab->PaintBox->ClientToScreen(TPoint(X, Y));
                ElementPopupMenu->Popup(popupPos.X, popupPos.Y);
                break;
            }
        }
    }
}

void __fastcall TMainForm::CircuitImageMouseMove(TObject *Sender, TShiftState Shift, int X, int Y) {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || !currentTab->ScrollBox) return;

    TPoint logicalPos = ScreenToLogical(TPoint(
        X + currentTab->ScrollBox->HorzScrollBar->Position,
        Y + currentTab->ScrollBox->VertScrollBar->Position
    ));

    // Оптимизация: обновляем только при значительном перемещении
    static int lastX = -1, lastY = -1;

	if (FIsDragging && FDraggedElement) {
		if (abs(logicalPos.X - lastX) > 1 || abs(logicalPos.Y - lastY) > 1) {
		int newLeft = logicalPos.X - FDragOffsetX;
		int newTop = logicalPos.Y - FDragOffsetY;

		// Привязка к сетке
		TPoint snappedPos = SnapToGridPoint(TPoint(newLeft, newTop));
		newLeft = snappedPos.X;
		newTop = snappedPos.Y;

		TRect newBounds = TRect(
			newLeft,
			newTop,
			newLeft + FDraggedElement->Bounds.Width(),
			newTop + FDraggedElement->Bounds.Height()
		);

		FDraggedElement->SetBounds(newBounds);
		// ПРИНУДИТЕЛЬНЫЙ ПЕРЕСЧЕТ ТОЧЕК СОЕДИНЕНИЯ
		FDraggedElement->CalculateRelativePositions();

		if (currentTab->PaintBox) {
			currentTab->PaintBox->Repaint();
		}

		lastX = logicalPos.X;
		lastY = logicalPos.Y;
		}
	}

    else if (FIsSelecting) {
        FSelectionRect.Right = X;
        FSelectionRect.Bottom = Y;
        if (currentTab->PaintBox) {
            currentTab->PaintBox->Repaint();
        }
    }

    // Обновление курсора
    if (btnConnectionMode->Down || FIsConnecting) {
        bool overConnection = false;
        for (auto& element : currentTab->Elements) {
            if (element->GetConnectionAt(logicalPos.X, logicalPos.Y)) {
                overConnection = true;
                break;
            }
        }
        if (currentTab->PaintBox) {
            currentTab->PaintBox->Cursor = overConnection ? crHandPoint : crCross;
        }
    } else {
        if (currentTab->PaintBox) {
            currentTab->PaintBox->Cursor = crDefault;
        }
    }
}

void __fastcall TMainForm::CircuitImageMouseUp(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y) {

    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || !currentTab->ScrollBox) return;

    if (Button == mbLeft) {
        if (FIsSelecting) {
            FIsSelecting = false;

            TRect normalizedRect = FSelectionRect;
            if (normalizedRect.Left > normalizedRect.Right) {
                std::swap(normalizedRect.Left, normalizedRect.Right);
            }
            if (normalizedRect.Top > normalizedRect.Bottom) {
                std::swap(normalizedRect.Top, normalizedRect.Bottom);
            }

            // Преобразуем экранный прямоугольник в логические координаты
            TPoint logicalTopLeft = ScreenToLogical(TPoint(
                normalizedRect.Left + currentTab->ScrollBox->HorzScrollBar->Position,
                normalizedRect.Top + currentTab->ScrollBox->VertScrollBar->Position
            ));
            TPoint logicalBottomRight = ScreenToLogical(TPoint(
                normalizedRect.Right + currentTab->ScrollBox->HorzScrollBar->Position,
                normalizedRect.Bottom + currentTab->ScrollBox->VertScrollBar->Position
            ));
            TRect logicalSelectionRect(logicalTopLeft, logicalBottomRight);

            for (auto& element : currentTab->Elements) {
                TRect bounds = element->Bounds;
                if (bounds.Left <= logicalSelectionRect.Right && bounds.Right >= logicalSelectionRect.Left &&
                    bounds.Top <= logicalSelectionRect.Bottom && bounds.Bottom >= logicalSelectionRect.Top) {

                    auto it = std::find(FSelectedElements.begin(), FSelectedElements.end(), element.get());
                    if (it == FSelectedElements.end()) {
                        FSelectedElements.push_back(element.get());
                    }
                }
            }

            if (FSelectedElements.size() == 1) {
                FSelectedElement = FSelectedElements[0];
            }

            StatusBar->Panels->Items[0]->Text = "Выделено элементов: " + IntToStr(static_cast<int>(FSelectedElements.size()));
            if (currentTab->PaintBox) {
                currentTab->PaintBox->Repaint();
            }
        } else {
            FIsDragging = false;
            FDraggedElement = nullptr;
        }
    }
}

void __fastcall TMainForm::btnRunSimulationClick(TObject *Sender) {
    if (!FSimulationRunning) {
        FSimulationRunning = true;
        FSimulationTimer->Enabled = true;
        btnRunSimulation->Caption = "Стоп";
        StatusBar->Panels->Items[0]->Text = "Симуляция запущена";
    } else {
        FSimulationRunning = false;
        FSimulationTimer->Enabled = false;
        btnRunSimulation->Caption = "Симуляция";
        StatusBar->Panels->Items[0]->Text = "Симуляция остановлена. Шагов: " + IntToStr(FSimulationStep);
    }
}

void TMainForm::RunSimulationStep() {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab) return;

    for (auto& connection : currentTab->Connections) {
        if (connection.first && connection.second) {
            connection.second->Value = connection.first->Value;
        }
    }

    for (auto& element : currentTab->Elements) {
        element->Calculate();
    }

    FSimulationStep++;

    // Перерисовываем текущую вкладку
    if (currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
}

void __fastcall TMainForm::SimulationTimerTimer(TObject* Sender) {
    RunSimulationStep();

    // Визуальная индикация работы симуляции
    StatusBar->Panels->Items[0]->Text =
        "Симуляция выполняется... Шаг: " + IntToStr(FSimulationStep);
}

void __fastcall TMainForm::btnResetSimulationClick(TObject *Sender) {
    FSimulationRunning = false;
    FSimulationTimer->Enabled = false;
    FSimulationStep = 0;
    btnRunSimulation->Caption = "Симуляция";
    ResetSimulation();

    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
    StatusBar->Panels->Items[0]->Text = "Симуляция сброшена.";
}

void TMainForm::ResetSimulation() {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab) return;

    for (auto& element : currentTab->Elements) {
        TTernaryTrigger* trigger = dynamic_cast<TTernaryTrigger*>(element.get());
        if (trigger) {
            trigger->Reset();
        }

        TCounter* counter = dynamic_cast<TCounter*>(element.get());
        if (counter) {
            counter->Reset();
        }

        for (auto& input : element->Inputs) {
            input.Value = TTernary::ZERO;
        }
        for (auto& output : element->Outputs) {
            output.Value = TTernary::ZERO;
        }
    }

    for (auto& connection : currentTab->Connections) {
        if (connection.first) connection.first->Value = TTernary::ZERO;
        if (connection.second) connection.second->Value = TTernary::ZERO;
    }
}

void __fastcall TMainForm::btnConnectionModeClick(TObject *Sender) {
    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->IsReadOnly) {
        StatusBar->Panels->Items[0]->Text = "Режим просмотра: создание соединений запрещено";
        btnConnectionMode->Down = false;
        return;
    }

    if (btnConnectionMode->Down) {
        StatusBar->Panels->Items[0]->Text = "Режим соединения: выбирайте точки для соединения элементов.";
        TTabData* currentTab = GetCurrentTabData();
        if (currentTab && currentTab->PaintBox) {
            currentTab->PaintBox->Cursor = crCross;
        }
    } else {
        StatusBar->Panels->Items[0]->Text = "Режим выбора: выбирайте и перемещайте элементы.";
        TTabData* currentTab = GetCurrentTabData();
        if (currentTab && currentTab->PaintBox) {
            currentTab->PaintBox->Cursor = crDefault;
        }
        FIsConnecting = false;
        FConnectionStart = nullptr;
    }
}

void __fastcall TMainForm::miPropertiesClick(TObject *Sender) {
    if (FSelectedElement) {
        ShowElementProperties(FSelectedElement);
    }
}

void TMainForm::ShowElementProperties(TCircuitElement* Element) {
    String info = "Свойства элемента:\n";
    info += "Тип: " + Element->Name + "\n";
    info += "ID: " + IntToStr(static_cast<int>(Element->Id)) + "\n";
    info += "Позиция: (" + IntToStr(static_cast<int>(Element->Bounds.Left)) + ", " +
            IntToStr(static_cast<int>(Element->Bounds.Top)) + ")\n";
    info += "Размер: " + IntToStr(static_cast<int>(Element->Bounds.Width())) + " x " +
            IntToStr(static_cast<int>(Element->Bounds.Height())) + "\n";
    info += "Входы: " + IntToStr(static_cast<int>(Element->Inputs.size())) + "\n";
    info += "Выходы: " + IntToStr(static_cast<int>(Element->Outputs.size())) + "\n";

    String stateStr;
    switch (Element->CurrentState) {
        case TTernary::NEG: stateStr = "-1"; break;
        case TTernary::ZERO: stateStr = "0"; break;
        case TTernary::POS: stateStr = "+1"; break;
    }
    info += "Текущее состояние: " + stateStr;

    Application->MessageBox(info.w_str(), L"Свойства элемента", MB_OK | MB_ICONINFORMATION);
}

void __fastcall TMainForm::miRotateElementClick(TObject *Sender) {
    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->IsReadOnly) {
        StatusBar->Panels->Items[0]->Text = "Режим просмотра: вращение элементов запрещено";
        return;
    }

    if (FSelectedElement) {
        TRect newBounds = FSelectedElement->Bounds;
        int width = newBounds.Width();
        int height = newBounds.Height();

        newBounds.Right = newBounds.Left + height;
        newBounds.Bottom = newBounds.Top + width;

        FSelectedElement->SetBounds(newBounds);
        UpdatePaintBoxSize();
        TTabData* currentTab = GetCurrentTabData();
        if (currentTab && currentTab->PaintBox) {
            currentTab->PaintBox->Repaint();
        }
        StatusBar->Panels->Items[0]->Text = "Элемент повернут.";
    }
}
void __fastcall TMainForm::btnZoomInClick(TObject *Sender) {
    FZoomFactor *= 1.2;
    if (FZoomFactor > 5.0) FZoomFactor = 5.0;

    ApplyZoom();
    StatusBar->Panels->Items[0]->Text = "Увеличение: " + FloatToStrF(FZoomFactor * 100, ffFixed, 3, 0) + "%";
}

void __fastcall TMainForm::btnZoomOutClick(TObject *Sender) {
    FZoomFactor /= 1.2;
    if (FZoomFactor < 0.1) FZoomFactor = 0.1;

    ApplyZoom();
    StatusBar->Panels->Items[0]->Text = "Увеличение: " + FloatToStrF(FZoomFactor * 100, ffFixed, 3, 0) + "%";
}

void __fastcall TMainForm::btnZoomFitClick(TObject *Sender) {
    FZoomFactor = 1.0;
    ApplyZoom();
    CenterCircuit();
    StatusBar->Panels->Items[0]->Text = "Увеличение сброшено до 100%";
}

void TMainForm::ApplyZoom() {
    UpdatePaintBoxSize();
    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
}

void __fastcall TMainForm::FormResize(TObject *Sender) {
    UpdatePaintBoxSize();
    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
}

void __fastcall TMainForm::WorkspaceResize(TObject *Sender) {
    UpdatePaintBoxSize();
    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
}

void TMainForm::UpdatePaintBoxSize() {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || !currentTab->ScrollBox) return;

    TRect bounds = GetCircuitBounds();

    int padding = 400;
    int newWidth = std::max(currentTab->ScrollBox->ClientWidth, bounds.Width() + padding);
    int newHeight = std::max(currentTab->ScrollBox->ClientHeight, bounds.Height() + padding);

    if (currentTab->PaintBox) {
        currentTab->PaintBox->Width = static_cast<int>(newWidth * FZoomFactor);
        currentTab->PaintBox->Height = static_cast<int>(newHeight * FZoomFactor);
    }
}

void TMainForm::CenterCircuit() {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || !currentTab->ScrollBox || currentTab->Elements.empty()) return;

    TRect bounds = GetCircuitBounds();

    // Вычисляем центр схемы
    int centerX = (bounds.Left + bounds.Right) / 2;
    int centerY = (bounds.Top + bounds.Bottom) / 2;

    // Вычисляем центр видимой области в логических координатах
    int visibleWidth = currentTab->ScrollBox->ClientWidth / FZoomFactor;
    int visibleHeight = currentTab->ScrollBox->ClientHeight / FZoomFactor;

    int visibleCenterX = currentTab->ScrollBox->HorzScrollBar->Position / FZoomFactor + visibleWidth / 2;
    int visibleCenterY = currentTab->ScrollBox->VertScrollBar->Position / FZoomFactor + visibleHeight / 2;

    // Вычисляем новое положение для центрирования
    int newScrollX = (centerX - visibleWidth / 2) * FZoomFactor;
    int newScrollY = (centerY - visibleHeight / 2) * FZoomFactor;

    // Ограничиваем значения скролла
    newScrollX = std::max(0, newScrollX);
    newScrollY = std::max(0, newScrollY);
    newScrollX = std::min(newScrollX, currentTab->ScrollBox->HorzScrollBar->Range - visibleWidth);
    newScrollY = std::min(newScrollY, currentTab->ScrollBox->VertScrollBar->Range - visibleHeight);

    currentTab->ScrollBox->HorzScrollBar->Position = newScrollX;
    currentTab->ScrollBox->VertScrollBar->Position = newScrollY;

    if (currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
}

TRect TMainForm::GetCircuitBounds() {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab || currentTab->Elements.empty()) {
        return TRect(0, 0, 800, 600);
    }

    TRect bounds = currentTab->Elements[0]->Bounds;
    for (auto& element : currentTab->Elements) {
        bounds.Left = std::min(bounds.Left, element->Bounds.Left);
        bounds.Top = std::min(bounds.Top, element->Bounds.Top);
        bounds.Right = std::max(bounds.Right, element->Bounds.Right);
        bounds.Bottom = std::max(bounds.Bottom, element->Bounds.Bottom);
    }

    return bounds;
}

// ИСПРАВЛЕННЫЕ МЕТОДЫ ДЛЯ УПРАВЛЕНИЯ ВКЛАДКАМИ

TTabSheet* TMainForm::CreateNewTab(const String& Title, TSubCircuit* SubCircuit) {
    TTabSheet* newTab = new TTabSheet(SchemePageControl);
    newTab->PageControl = SchemePageControl;
    newTab->Caption = Title;

    // Создаем контейнер для данных вкладки
    TTabData* tabData = new TTabData();
    tabData->ScrollBox = new TScrollBox(newTab);
    tabData->ScrollBox->Parent = newTab;
    tabData->ScrollBox->Align = alClient;
    tabData->ScrollBox->OnMouseWheel = WorkspaceMouseWheel;
    tabData->ScrollBox->OnResize = WorkspaceResize;

    tabData->PaintBox = new TPaintBox(tabData->ScrollBox);
    tabData->PaintBox->Parent = tabData->ScrollBox;
    tabData->PaintBox->Align = alClient;
    tabData->PaintBox->OnMouseDown = CircuitImageMouseDown;
    tabData->PaintBox->OnMouseMove = CircuitImageMouseMove;
    tabData->PaintBox->OnMouseUp = CircuitImageMouseUp;
    tabData->PaintBox->OnPaint = CircuitImagePaint;

    // Инициализируем данные вкладки
    if (SubCircuit) {
        // Для вкладки подсхемы копируем внутренние элементы и соединения
        const auto& internalElements = SubCircuit->GetInternalElements();
        const auto& internalConnections = SubCircuit->GetInternalConnections();

        // Создаем карту для соответствия старых указателей на новые элементы
        std::map<TCircuitElement*, TCircuitElement*> elementMap;

        // Копируем элементы
        for (const auto& element : internalElements) {
            // Создаем копию элемента
            auto newElement = CreateElementByClassName(element->GetClassName(), tabData->NextElementId++,
                                                     element->Bounds.Left, element->Bounds.Top);
            if (newElement) {
                newElement->SetBounds(element->Bounds);

                // Копируем состояние
                newElement->SetCurrentState(element->CurrentState);

                // Копируем входы и выходы
                for (size_t i = 0; i < std::min(element->Inputs.size(), newElement->Inputs.size()); ++i) {
                    newElement->Inputs[i].Value = element->Inputs[i].Value;
                    newElement->Inputs[i].RelX = element->Inputs[i].RelX;
                    newElement->Inputs[i].RelY = element->Inputs[i].RelY;
                }
                for (size_t i = 0; i < std::min(element->Outputs.size(), newElement->Outputs.size()); ++i) {
                    newElement->Outputs[i].Value = element->Outputs[i].Value;
                    newElement->Outputs[i].RelX = element->Outputs[i].RelX;
                    newElement->Outputs[i].RelY = element->Outputs[i].RelY;
                }

                // Сохраняем соответствие
                elementMap[element.get()] = newElement.get();
                tabData->Elements.push_back(std::move(newElement));
            }
        }

        // Копируем соединения
        for (const auto& conn : internalConnections) {
            if (conn.first && conn.second) {
                TCircuitElement* fromElement = elementMap[conn.first->Owner];
                TCircuitElement* toElement = elementMap[conn.second->Owner];

                if (fromElement && toElement) {
                    // Находим соответствующие точки соединения в новых элементах
                    TConnectionPoint* fromPoint = FindConnectionPointByRelCoords(fromElement, conn.first->RelX, conn.first->RelY, false);
                    TConnectionPoint* toPoint = FindConnectionPointByRelCoords(toElement, conn.second->RelX, conn.second->RelY, true);

                    if (fromPoint && toPoint) {
                        tabData->Connections.push_back(std::make_pair(fromPoint, toPoint));
                    }
                }
            }
        }

        tabData->IsSubCircuit = true;
        tabData->SubCircuit = SubCircuit;
        tabData->IsReadOnly = true; // Режим только для чтения
        SubCircuit->SetAssociatedTab(newTab);
    } else {
        // Основная схема
        tabData->IsSubCircuit = false;
        tabData->IsReadOnly = false;
    }

    newTab->Tag = NativeInt(tabData);

    // Обновляем ширину вкладок после создания новой
    UpdateTabWidths();

    return newTab;
}

void TMainForm::UpdateCurrentTab() {
    // При переключении вкладок обновляем выделение и прочее
    FSelectedElements.clear();
    FSelectedElement = nullptr;
    FIsConnecting = false;
    FConnectionStart = nullptr;
}

// ИСПРАВЛЕННАЯ ВЕРСИЯ - добавлен const квалификатор
TTabData* TMainForm::GetCurrentTabData() const {
    if (SchemePageControl->ActivePage && SchemePageControl->ActivePage->Tag != 0) {
        return reinterpret_cast<TTabData*>(SchemePageControl->ActivePage->Tag);
    }
    return nullptr;
}

void __fastcall TMainForm::SchemePageControlChange(TObject *Sender) {
    UpdateCurrentTab();
}

void __fastcall TMainForm::miCloseTabClick(TObject *Sender) {
    if (SchemePageControl->PageCount > 1 && SchemePageControl->ActivePage) {
        TTabSheet* activeTab = SchemePageControl->ActivePage;
        TTabData* tabData = reinterpret_cast<TTabData*>(activeTab->Tag);

        if (tabData) {
            // Если это вкладка подсхемы, обновляем данные в подсхеме перед закрытием
            if (tabData->IsSubCircuit && tabData->SubCircuit) {
                TSubCircuit* subCircuit = dynamic_cast<TSubCircuit*>(tabData->SubCircuit);
                if (subCircuit) {
                    subCircuit->SetAssociatedTab(nullptr);
                }
            }

            delete tabData;
        }

        delete activeTab;

        // Обновляем ширину вкладок после закрытия
        UpdateTabWidths();
    }
}

void TMainForm::CloseTab(TTabSheet* Tab) {
    if (SchemePageControl->PageCount > 1 && Tab) {
        TTabData* tabData = reinterpret_cast<TTabData*>(Tab->Tag);
        if (tabData) {
            delete tabData;
        }

        // Удаляем вкладку из PageControl перед физическим удалением
        Tab->PageControl = nullptr;
        delete Tab;

        // Сбрасываем состояние наведения
        FHotCloseTabIndex = -1;

        // Обновляем ширину вкладок после закрытия
        UpdateTabWidths();
    }
}

void TMainForm::UpdateTabTitle(TTabSheet* Tab, const String& Title) {
    if (Tab) {
        Tab->Caption = Title;
    }
}

void __fastcall TMainForm::miExitClick(TObject *Sender) {
    Close();
}

void __fastcall TMainForm::miAboutClick(TObject *Sender) {
    String aboutText =
        L"Setun IDE - Среда разработки для троичной логики\n"
        L"Основано на книге 1965 года\n"
        L"Тимошенко А.Н. 2022 год\n\n"
        L"Модульная архитектура - поддержка загружаемых библиотек элементов";

    Application->MessageBox(aboutText.w_str(), L"О программе", MB_OK | MB_ICONINFORMATION);
}

// ИСПРАВЛЕННЫЕ ОБРАБОТЧИКИ СОХРАНЕНИЯ И ЗАГРУЗКИ

void __fastcall TMainForm::btnSaveSchemeClick(TObject *Sender) {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab) return;

    if (SaveDialog->Execute()) {
        try {
            SaveSchemeToFile(SaveDialog->FileName, currentTab);
            StatusBar->Panels->Items[0]->Text = "Схема сохранена: " + SaveDialog->FileName;
        }
        catch (Exception &e) {
            Application->MessageBox(L"Ошибка при сохранении схемы", L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }
}

void __fastcall TMainForm::btnLoadSchemeClick(TObject *Sender) {
    if (OpenDialog->Execute()) {
        try {
            if (Application->MessageBox(L"Загрузить новую схему? Текущая схема будет потеряна.",
                L"Подтверждение", MB_YESNO | MB_ICONQUESTION) == ID_YES) {
                TTabData* currentTab = GetCurrentTabData();
                if (currentTab) {
                    LoadSchemeFromFile(OpenDialog->FileName, currentTab);
                    StatusBar->Panels->Items[0]->Text = "Схема загружена: " + OpenDialog->FileName;
                }
            }
        }
        catch (Exception &e) {
            Application->MessageBox(L"Ошибка при загрузке схемы", L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }
}
// ИСПРАВЛЕННЫЕ МЕТОДЫ ДЛЯ РАБОТЫ С ВЫДЕЛЕННЫМИ ЭЛЕМЕНТАМИ

std::vector<TCircuitElement*> TMainForm::GetSelectedElements() {
    return FSelectedElements;
}

void TMainForm::CreateSubCircuitFromSelection() {
    auto selectedElements = GetSelectedElements();

    if (selectedElements.size() < 2) {
        StatusBar->Panels->Items[0]->Text = "Выберите хотя бы 2 элемента для группировки";
        return;
    }

    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab) return;

    TRect totalBounds = selectedElements[0]->Bounds;
    for (auto element : selectedElements) {
        totalBounds.Left = std::min(totalBounds.Left, element->Bounds.Left);
        totalBounds.Top = std::min(totalBounds.Top, element->Bounds.Top);
        totalBounds.Right = std::max(totalBounds.Right, element->Bounds.Right);
        totalBounds.Bottom = std::max(totalBounds.Bottom, element->Bounds.Bottom);
    }

    int centerX = (totalBounds.Left + totalBounds.Right) / 2;
    int centerY = (totalBounds.Top + totalBounds.Bottom) / 2;

    // Собираем внутренние соединения
    std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>> internalConnections;
    for (auto& connection : currentTab->Connections) {
        bool fromSelected = false;
        bool toSelected = false;

        for (auto element : selectedElements) {
            if (connection.first->Owner == element) fromSelected = true;
            if (connection.second->Owner == element) toSelected = true;
        }

        if (fromSelected && toSelected) {
            internalConnections.push_back(connection);
        }
    }

    // Собираем элементы для подсхемы
    std::vector<std::unique_ptr<TCircuitElement>> subCircuitElements;
    for (auto selectedElement : selectedElements) {
        auto it = std::find_if(currentTab->Elements.begin(), currentTab->Elements.end(),
            [selectedElement](const std::unique_ptr<TCircuitElement>& elem) {
                return elem.get() == selectedElement;
            });

        if (it != currentTab->Elements.end()) {
            subCircuitElements.push_back(std::move(*it));
        }
    }

    // ИСПРАВЛЕННАЯ ЛЯМБДА - используем get() для сравнения с nullptr
    currentTab->Elements.erase(std::remove_if(currentTab->Elements.begin(), currentTab->Elements.end(),
        [](const std::unique_ptr<TCircuitElement>& elem) {
            return elem.get() == nullptr;
        }), currentTab->Elements.end());

    // Удаляем внутренние соединения из основного списка
    for (auto& internalConn : internalConnections) {
        auto it = std::find(currentTab->Connections.begin(), currentTab->Connections.end(), internalConn);
        if (it != currentTab->Connections.end()) {
            currentTab->Connections.erase(it);
        }
    }

    // Создаем подсхему
    auto subCircuit = std::make_unique<TSubCircuit>(currentTab->NextElementId++, centerX, centerY,
                                                   std::move(subCircuitElements),
                                                   internalConnections);

    FSelectedElement = subCircuit.get();
    FSelectedElements.clear();
    FSelectedElements.push_back(subCircuit.get());

    currentTab->Elements.push_back(std::move(subCircuit));

    UpdatePaintBoxSize();
    if (currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
    StatusBar->Panels->Items[0]->Text = "Создана подсхема из " + IntToStr(static_cast<int>(selectedElements.size())) + " элементов";
}

TConnectionPoint* TMainForm::FindRestoredConnectionPoint(const TConnectionPoint* originalPoint) {
    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab) return nullptr;

    // Ищем восстановленный элемент по оригинальным координатам и имени
    for (auto& element : currentTab->Elements) {
        if (element->Bounds.Left == originalPoint->Owner->Bounds.Left &&
            element->Bounds.Top == originalPoint->Owner->Bounds.Top &&
            element->Name == originalPoint->Owner->Name) {

            // Находим соответствующую точку соединения по относительным координатам
            if (originalPoint->IsInput) {
                for (auto& input : element->Inputs) {
                    if (abs(input.RelX - originalPoint->RelX) < 0.01 &&
                        abs(input.RelY - originalPoint->RelY) < 0.01) {
                        return &input;
                    }
                }
            } else {
                for (auto& output : element->Outputs) {
                    if (abs(output.RelX - originalPoint->RelX) < 0.01 &&
                        abs(output.RelY - originalPoint->RelY) < 0.01) {
                        return &output;
                    }
                }
            }
        }
    }
    return nullptr;
}

void TMainForm::UngroupSubCircuit(TCircuitElement* SubCircuit) {
    TSubCircuit* subCircuit = dynamic_cast<TSubCircuit*>(SubCircuit);
    if (!subCircuit) return;

    TTabData* currentTab = GetCurrentTabData();
    if (!currentTab) return;

    const auto& internalElements = subCircuit->GetInternalElements();
    const auto& internalConnections = subCircuit->GetInternalConnections();

    // Восстанавливаем элементы в основной схеме
    for (const auto& element : internalElements) {
        // Восстанавливаем оригинальные позиции элементов
        TRect originalBounds = element->Bounds;

        // Создаем новый элемент того же типа
        auto newElement = CreateElementByClassName(element->GetClassName(), currentTab->NextElementId++, originalBounds.Left, originalBounds.Top);
        if (newElement) {
            newElement->SetBounds(originalBounds);
            newElement->CalculateRelativePositions();

            // Восстанавливаем состояния
            for (size_t i = 0; i < std::min(element->Inputs.size(), newElement->Inputs.size()); ++i) {
                newElement->Inputs[i].Value = element->Inputs[i].Value;
                newElement->Inputs[i].RelX = element->Inputs[i].RelX;
                newElement->Inputs[i].RelY = element->Inputs[i].RelY;
            }
            for (size_t i = 0; i < std::min(element->Outputs.size(), newElement->Outputs.size()); ++i) {
                newElement->Outputs[i].Value = element->Outputs[i].Value;
                newElement->Outputs[i].RelX = element->Outputs[i].RelX;
                newElement->Outputs[i].RelY = element->Outputs[i].RelY;
            }

            currentTab->Elements.push_back(std::move(newElement));
        }
    }

    // Восстанавливаем ВСЕ соединения
    for (const auto& conn : internalConnections) {
        TConnectionPoint* fromPoint = FindRestoredConnectionPoint(conn.first);
        TConnectionPoint* toPoint = FindRestoredConnectionPoint(conn.second);

        if (fromPoint && toPoint) {
            currentTab->Connections.push_back(std::make_pair(fromPoint, toPoint));
        }
    }

    // Удаляем подсхему
    auto it = std::find_if(currentTab->Elements.begin(), currentTab->Elements.end(),
        [SubCircuit](const std::unique_ptr<TCircuitElement>& elem) {
            return elem.get() == SubCircuit;
        });

    if (it != currentTab->Elements.end()) {
        currentTab->Elements.erase(it);
    }

    FSelectedElement = nullptr;
    FSelectedElements.clear();

    UpdatePaintBoxSize();
    if (currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
    StatusBar->Panels->Items[0]->Text = "Подсхема разгруппирована";
}

void __fastcall TMainForm::btnGroupElementsClick(TObject *Sender) {
    CreateSubCircuitFromSelection();
}

void __fastcall TMainForm::btnUngroupElementsClick(TObject *Sender) {
    if (FSelectedElement) {
        TSubCircuit* subCircuit = dynamic_cast<TSubCircuit*>(FSelectedElement);
        if (subCircuit) {
            UngroupSubCircuit(subCircuit);
        } else {
            StatusBar->Panels->Items[0]->Text = "Выбранный элемент не является подсхемой";
        }
    } else {
        StatusBar->Panels->Items[0]->Text = "Не выбрана подсхема для разгруппировки";
    }
}

void __fastcall TMainForm::miViewSubCircuitClick(TObject *Sender) {
    if (FSelectedElement) {
        TSubCircuit* subCircuit = dynamic_cast<TSubCircuit*>(FSelectedElement);
        if (subCircuit) {
            // Создаем новую вкладку для просмотра подсхемы
            String tabName = "Просмотр: " + subCircuit->Name;
            TTabSheet* viewTab = CreateNewTab(tabName, subCircuit);
            SchemePageControl->ActivePage = viewTab;

            StatusBar->Panels->Items[0]->Text = "Открыт просмотр подсхемы";
        } else {
            StatusBar->Panels->Items[0]->Text = "Выбранный элемент не является подсхемой";
        }
    }
}

// ============================================================================
// НОВЫЕ МЕТОДЫ ДЛЯ УЛУЧШЕНИЙ
// ============================================================================

// Метод для автоматической установки ширины вкладок
void TMainForm::UpdateTabWidths() {
    if (!SchemePageControl || SchemePageControl->PageCount == 0) return;

    TCanvas* canvas = SchemePageControl->Canvas;
    canvas->Font->Assign(SchemePageControl->Font);

    int minTabWidth = 120;
    int maxTabWidth = 200;
    int padding = 40;

    int maxWidth = minTabWidth;

    for (int i = 0; i < SchemePageControl->PageCount; i++) {
        String tabText = SchemePageControl->Pages[i]->Caption;
        int textWidth = canvas->TextWidth(tabText);
        int calculatedWidth = textWidth + padding;

        if (calculatedWidth > maxWidth && calculatedWidth <= maxTabWidth) {
            maxWidth = calculatedWidth;
        } else if (calculatedWidth > maxTabWidth) {
            maxWidth = maxTabWidth;
        }
    }

    SchemePageControl->TabWidth = maxWidth;
}

// Поиск точки соединения по относительным координатам
TConnectionPoint* TMainForm::FindConnectionPointByRelCoords(TCircuitElement* element, double relX, double relY, bool isInput) {
    if (!element) return nullptr;

    auto& points = isInput ? element->Inputs : element->Outputs;

    for (auto& point : points) {
        if (fabs(point.RelX - relX) < 0.001 && fabs(point.RelY - relY) < 0.001) {
            return &point;
        }
    }

    return nullptr;
}

// Метод для получения области крестика на вкладке
TRect TMainForm::GetTabCloseButtonRect(TCustomTabControl *Control, int TabIndex, const TRect &TabRect) {
    int closeButtonSize = 12;
    int margin = 8;

    TRect closeRect;
    closeRect.Left = TabRect.Right - closeButtonSize - margin;
    closeRect.Right = closeRect.Left + closeButtonSize;
    closeRect.Top = TabRect.Top + (TabRect.Height() - closeButtonSize) / 2;
    closeRect.Bottom = closeRect.Top + closeButtonSize;

    return closeRect;
}

// Метод для проверки, находится ли точка в области крестика
bool TMainForm::IsPointInTabCloseButton(TCustomTabControl *Control, int TabIndex, int X, int Y) {
    TRect tabRect = Control->TabRect(TabIndex);
    TRect closeRect = GetTabCloseButtonRect(Control, TabIndex, tabRect);

    return (X >= closeRect.Left && X <= closeRect.Right &&
            Y >= closeRect.Top && Y <= closeRect.Bottom);
}

// Привязка точки к сетке
TPoint TMainForm::SnapToGridPoint(const TPoint& Point) {
    if (!FSnapToGrid) return Point;

    int gridSize = 20;
    // Правильная привязка с округлением до ближайшей сетки
    int snappedX = ((Point.X + gridSize/2) / gridSize) * gridSize;
    int snappedY = ((Point.Y + gridSize/2) / gridSize) * gridSize;

    return TPoint(snappedX, snappedY);
}

// Расчет пути для прямоугольного соединения
std::vector<TPoint> TMainForm::CalculateRectangularPath(const TPoint& Start, const TPoint& End) {
    std::vector<TPoint> path;

    // Используем точные координаты точек (они уже привязаны к сетке)
    TPoint snappedStart = Start;
    TPoint snappedEnd = End;

    path.push_back(snappedStart);

    // Горизонтальный отрезок от начальной точки
    int firstX = snappedStart.X + (snappedEnd.X > snappedStart.X ? 40 : -40);
    path.push_back(TPoint(firstX, snappedStart.Y));

    // Вертикальный отрезок к уровню конечной точки
    path.push_back(TPoint(firstX, snappedEnd.Y));

    // Горизонтальный отрезок к конечной точке
    path.push_back(snappedEnd);

    return path;
}

// Отрисовка прямоугольного соединения
void TMainForm::DrawRectangularConnection(TCanvas* Canvas, const TPoint& Start, const TPoint& End, TColor Color) {
    auto path = CalculateRectangularPath(Start, End);

    if (path.size() < 2) return;

    Canvas->Pen->Color = Color;
    Canvas->MoveTo(path[0].X, path[0].Y);

    for (size_t i = 1; i < path.size(); i++) {
        Canvas->LineTo(path[i].X, path[i].Y);
    }

    // Рисуем стрелку в конце
    if (path.size() >= 2) {
        TPoint lastSegmentStart = path[path.size() - 2];
        TPoint arrowTip = path[path.size() - 1];

        int dx = arrowTip.X - lastSegmentStart.X;
        int dy = arrowTip.Y - lastSegmentStart.Y;
        double length = sqrt(dx*dx + dy*dy);

        if (length > 10) {
            double unitX = dx / length;
            double unitY = dy / length;

            int arrowSize = 8;
            int arrowX = arrowTip.X - static_cast<int>(unitX * arrowSize);
            int arrowY = arrowTip.Y - static_cast<int>(unitY * arrowSize);

            Canvas->MoveTo(arrowX - static_cast<int>(unitY * arrowSize/2),
                          arrowY + static_cast<int>(unitX * arrowSize/2));
            Canvas->LineTo(arrowTip.X, arrowTip.Y);
            Canvas->LineTo(arrowX + static_cast<int>(unitY * arrowSize/2),
                          arrowY - static_cast<int>(unitX * arrowSize/2));
        }
    }
}

// Обработчики новых пунктов меню
void __fastcall TMainForm::miRectangularConnectionsClick(TObject *Sender) {
    FRectangularConnections = !FRectangularConnections;
    miRectangularConnections->Checked = FRectangularConnections;

    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }

    StatusBar->Panels->Items[0]->Text = FRectangularConnections ?
        "Прямоугольные соединения включены" : "Прямоугольные соединения выключены";
}

void __fastcall TMainForm::miSnapToGridClick(TObject *Sender) {
    FSnapToGrid = !FSnapToGrid;
    miSnapToGrid->Checked = FSnapToGrid;
    StatusBar->Panels->Items[0]->Text = FSnapToGrid ?
        "Привязка к сетке включена" : "Привязка к сетке выключена";
}

void __fastcall TMainForm::SchemePageControlDrawTab(TCustomTabControl *Control,
    int TabIndex, const TRect &Rect, bool Active) {

    // Приводим к TPageControl, так как SchemePageControl именно этого типа
    TPageControl* pageControl = dynamic_cast<TPageControl*>(Control);
    if (!pageControl || TabIndex < 0 || TabIndex >= pageControl->PageCount) return;

    TCanvas* canvas = pageControl->Canvas;
    TTabSheet* tab = pageControl->Pages[TabIndex];

    if (!tab) return;

    // Фон вкладки
    if (Active) {
        canvas->Brush->Color = clHighlight;
        canvas->Font->Color = clHighlightText;
    } else {
        canvas->Brush->Color = clBtnFace;
        canvas->Font->Color = clWindowText;
    }
    canvas->FillRect(Rect);

    // Текст вкладки
    String tabText = tab->Caption;
    TRect textRect = Rect;
    textRect.Left += 8;
    textRect.Right -= 25; // Место для крестика

    int textHeight = canvas->TextHeight(tabText);
    int textY = textRect.Top + (textRect.Height() - textHeight) / 2;
    canvas->TextRect(textRect, textRect.Left, textY, tabText);

    // Крестик для закрытия
    TRect closeRect = GetTabCloseButtonRect(pageControl, TabIndex, Rect);

    // Фон крестика
    if (FHotCloseTabIndex == TabIndex) {
        canvas->Brush->Color = clRed;
        canvas->FillRect(closeRect);
    } else {
        canvas->Brush->Color = Active ? clHighlight : clBtnFace;
        canvas->FillRect(closeRect);
    }

    // Рисуем крестик
    canvas->Pen->Color = Active ? clHighlightText : clWindowText;
    canvas->Pen->Width = 1;

    int crossSize = 6;
    int centerX = closeRect.Left + closeRect.Width() / 2;
    int centerY = closeRect.Top + closeRect.Height() / 2;

    canvas->MoveTo(centerX - crossSize, centerY - crossSize);
    canvas->LineTo(centerX + crossSize, centerY + crossSize);
    canvas->MoveTo(centerX + crossSize, centerY - crossSize);
    canvas->LineTo(centerX - crossSize, centerY + crossSize);
}

void __fastcall TMainForm::SchemePageControlMouseDown(TObject *Sender,
    TMouseButton Button, TShiftState Shift, int X, int Y) {

    if (Button == mbLeft) {
        TPageControl* pageControl = dynamic_cast<TPageControl*>(Sender);
        if (!pageControl) return;

        // Проверяем клик на крестик
        for (int i = 0; i < pageControl->PageCount; i++) {
            TRect tabRect = pageControl->TabRect(i);
            if (IsPointInTabCloseButton(pageControl, i, X, Y)) {
                // Закрываем вкладку
                CloseTab(pageControl->Pages[i]);
                break;
            }
        }
    }
}

void __fastcall TMainForm::SchemePageControlMouseMove(TObject *Sender,
    TShiftState Shift, int X, int Y) {

    TPageControl* pageControl = dynamic_cast<TPageControl*>(Sender);
    if (!pageControl) return;

    int oldHotIndex = FHotCloseTabIndex;
    FHotCloseTabIndex = -1;

    // Проверяем, над каким крестиком находится курсор
    for (int i = 0; i < pageControl->PageCount; i++) {
        TRect tabRect = pageControl->TabRect(i);
        if (IsPointInTabCloseButton(pageControl, i, X, Y)) {
            FHotCloseTabIndex = i;
            break;
        }
    }

    // Перерисовываем, если состояние изменилось
    if (oldHotIndex != FHotCloseTabIndex) {
        pageControl->Invalidate();
    }

    // Меняем курсор при наведении на крестик
    if (FHotCloseTabIndex >= 0) {
        pageControl->Cursor = crHandPoint;
    } else {
        pageControl->Cursor = crDefault;
    }
}

void TMainForm::StartWireDrawing(TConnectionPoint* StartPoint) {
    FIsDrawingWire = true;
    FWireStartPoint = StartPoint;
    FCurrentWirePoints.clear();
    FCurrentWirePoints.push_back(TPoint(StartPoint->X, StartPoint->Y));

    StatusBar->Panels->Items[0]->Text = "Режим рисования провода: кликайте по сетке для добавления точек, ПКМ для завершения";
}

void TMainForm::AddWirePoint(const TPoint& Point) {
    if (FIsDrawingWire) {
        FCurrentWirePoints.push_back(Point);
        TTabData* currentTab = GetCurrentTabData();
        if (currentTab && currentTab->PaintBox) {
            currentTab->PaintBox->Repaint();
        }
    }
}

void TMainForm::CompleteWireDrawing(TConnectionPoint* EndPoint) {
    if (FIsDrawingWire && FWireStartPoint && EndPoint && FCurrentWirePoints.size() >= 2) {
        TTabData* currentTab = GetCurrentTabData();
        if (currentTab) {
            // Создаем соединение через промежуточные точки
            for (size_t i = 0; i < FCurrentWirePoints.size() - 1; i++) {
                // Создаем временные точки соединения
                TConnectionPoint* tempFrom = new TConnectionPoint(
                    FCurrentWirePoints[i].X, FCurrentWirePoints[i].Y,
                    TTernary::ZERO, false, TLineStyle::INTERNAL_CONNECTION
                );
                TConnectionPoint* tempTo = new TConnectionPoint(
                    FCurrentWirePoints[i+1].X, FCurrentWirePoints[i+1].Y,
                    TTernary::ZERO, true, TLineStyle::INTERNAL_CONNECTION
                );
                currentTab->Connections.push_back(std::make_pair(tempFrom, tempTo));
            }

            // Основное соединение
            currentTab->Connections.push_back(std::make_pair(FWireStartPoint, EndPoint));
        }
    }

    FIsDrawingWire = false;
    FCurrentWirePoints.clear();
    FWireStartPoint = nullptr;

    TTabData* currentTab = GetCurrentTabData();
    if (currentTab && currentTab->PaintBox) {
        currentTab->PaintBox->Repaint();
    }
    StatusBar->Panels->Items[0]->Text = "Соединение завершено";
}

