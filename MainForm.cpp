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

// Реализация TSubCircuit с правильной семантикой перемещения
TSubCircuit::TSubCircuit(int AId, int X, int Y,
                        std::vector<std::unique_ptr<TCircuitElement>>&& Elements,
                        const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& Connections)
    : TCircuitElement(AId, "SubCircuit", X, Y) {

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

    // Собираем все точки соединений, которые должны стать внешними
    std::vector<const TConnectionPoint*> externalInputs;
    std::vector<const TConnectionPoint*> externalOutputs;

    for (const auto& element : FInternalElements) {
        // Проверяем входы
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

        // Проверяем выходы
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

    // Создаем внешние входы
    int inputSpacing = FBounds.Height() / (externalInputs.size() + 1);
    for (size_t i = 0; i < externalInputs.size(); i++) {
        int y = FBounds.Top + inputSpacing * (i + 1);
        FInputs.push_back(TConnectionPoint(this, FBounds.Left - 15, y,
            TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    }

    // Создаем внешние выходы
    int outputSpacing = FBounds.Height() / (externalOutputs.size() + 1);
    for (size_t i = 0; i < externalOutputs.size(); i++) {
        int y = FBounds.Top + outputSpacing * (i + 1);
        FOutputs.push_back(TConnectionPoint(this, FBounds.Right + 15, y,
            TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
    }

    // Если нет внешних соединений, создаем по одному входу и выходу
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

// Конструктор MainForm
__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner),
    FSelectedElement(nullptr), FDraggedElement(nullptr), FConnectionStart(nullptr),
    FIsConnecting(false), FIsDragging(false), FIsSelecting(false),
    FNextElementId(1), FDragOffsetX(0), FDragOffsetY(0), FZoomFactor(1.0),
    FScrollOffsetX(0), FScrollOffsetY(0), FStandardLibraryHandle(0) {

    // Включаем двойную буферизацию
    CircuitImage->ControlStyle = CircuitImage->ControlStyle << csOpaque;
    DoubleBuffered = true;

    // Инициализируем скроллбары
    Workspace->HorzScrollBar->Tracking = true;
    Workspace->VertScrollBar->Tracking = true;

    KeyPreview = true;
}

// Новые методы для преобразования координат
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

// Новые методы для размещения элементов
TPoint TMainForm::GetVisibleAreaCenter() const {
    // Получаем видимую область в логических координатах
    int logicalLeft = Workspace->HorzScrollBar->Position / FZoomFactor;
    int logicalTop = Workspace->VertScrollBar->Position / FZoomFactor;
    int logicalRight = (Workspace->HorzScrollBar->Position + Workspace->ClientWidth) / FZoomFactor;
    int logicalBottom = (Workspace->VertScrollBar->Position + Workspace->ClientHeight) / FZoomFactor;

    return TPoint(
        (logicalLeft + logicalRight) / 2,
        (logicalTop + logicalBottom) / 2
    );
}

bool TMainForm::FindFreeLocation(int& x, int& y, int width, int height) {
    int originalX = x;
    int originalY = y;
    int step = 40; // шаг смещения в логических пикселях

    for (int attempt = 0; attempt < 50; attempt++) {
        TRect newRect(x, y, x + width, y + height);
        bool collision = false;

        // Проверяем пересечение с существующими элементами
        for (const auto& element : FElements) {
            if (newRect.IntersectsWith(element->Bounds)) {
                collision = true;
                break;
            }
        }

        if (!collision) {
            return true; // Нашли свободное место
        }

        // Сдвигаем по спирали от центра
        int ring = (attempt / 4) + 1;
        switch (attempt % 4) {
            case 0: x = originalX + ring * step; break;  // вправо
            case 1: y = originalY + ring * step; break;  // вниз
            case 2: x = originalX - ring * step; break;  // влево
            case 3: y = originalY - ring * step; break;  // вверх
        }
    }

    // Если не нашли свободное место, оставляем исходную позицию
    return false;
}

TPoint TMainForm::GetBestPlacementPosition(int width, int height) {
    TPoint visibleCenter = GetVisibleAreaCenter();

    int x = visibleCenter.X - width / 2;
    int y = visibleCenter.Y - height / 2;

    // Если центр занят, ищем свободное место вокруг
    if (!FindFreeLocation(x, y, width, height)) {
        // Если не нашли свободное место, используем центр
        x = visibleCenter.X - width / 2;
        y = visibleCenter.Y - height / 2;
    }

    return TPoint(x, y);
}

// Метод для удаления выделенных элементов
void TMainForm::DeleteSelectedElements() {
    if (FSelectedElements.empty()) return;

    std::vector<TCircuitElement*> elementsToDelete = FSelectedElements;

    // Удаляем соединения, связанные с выделенными элементами
    auto it = FConnections.begin();
    while (it != FConnections.end()) {
        bool shouldRemove = false;
        for (auto* element : elementsToDelete) {
            if ((it->first->Owner == element) || (it->second->Owner == element)) {
                shouldRemove = true;
                break;
            }
        }
        if (shouldRemove) {
            it = FConnections.erase(it);
        } else {
            ++it;
        }
    }

    // Удаляем элементы из вектора
    for (auto* elementToDelete : elementsToDelete) {
        auto elemIt = std::find_if(FElements.begin(), FElements.end(),
            [elementToDelete](const std::unique_ptr<TCircuitElement>& elem) {
                return elem.get() == elementToDelete;
            });

        if (elemIt != FElements.end()) {
            FElements.erase(elemIt);
        }
    }

    // Очищаем выделение
    FSelectedElements.clear();
    FSelectedElement = nullptr;

    UpdatePaintBoxSize();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "Удалено элементов: " + IntToStr(static_cast<int>(elementsToDelete.size()));
}

// Обработчик нажатия клавиш
void __fastcall TMainForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
    if (Key == VK_DELETE) {
        DeleteSelectedElements();
        Key = 0;
    }
}

void __fastcall TMainForm::FormCreate(TObject *Sender) {
    // Инициализируем менеджер библиотек
    FLibraryManager = std::make_unique<TLibraryManager>();

    // Создаем встроенную базовую библиотеку
    CreateBasicLibrary();

    // Загружаем стандартную библиотеку из DLL
    if (!LoadStandardLibrary()) {
        Application->MessageBox(
            L"Не удалось загрузить стандартную библиотеку элементов из DLL.\nПриложение продолжит работу со встроенной библиотекой.",
            L"Предупреждение",
            MB_OK | MB_ICONWARNING
        );
    }

    CreateCompleteLibrary();
    CircuitImage->Align = alNone;
    CircuitImage->Cursor = crCross;
    UpdatePaintBoxSize();
    CenterCircuit();

    btnConnectionMode->AllowAllUp = true;
    btnConnectionMode->GroupIndex = 1;

    // Инициализируем селектор библиотек
    UpdateLibrarySelector();

    StatusBar->Panels->Items[0]->Text = "Готов к работе. Выберите элемент из библиотеки или режим соединения.";
    Caption = "Setun IDE - Троичная логика по книге 1965 года";
}

void TMainForm::CreateBasicLibrary() {
    // Создаем встроенную базовую библиотеку
    FBasicLibrary = std::make_unique<TComponentLibrary>("Basic", "Базовая библиотека элементов", "1.0");

    // Регистрируем базовые элементы с простыми именами
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

    // Регистрируем библиотеку в менеджере
    FLibraryManager->RegisterLibrary(std::move(FBasicLibrary));
}


bool TMainForm::LoadStandardLibrary() {
    String currentDir = GetCurrentDir();
    String dllPath = currentDir + "\\StandardLibrary.dll";

    if (!FileExists(dllPath)) {
        return false;
    }

    FStandardLibraryHandle = LoadLibrary(dllPath.w_str());

    if (!FStandardLibraryHandle) {
        return false;
    }

    TRegisterLibraryFunction registerFunc = nullptr;

    // Пробуем разные варианты имен функции
    const char* functionNames[] = {
        "RegisterStandardLibrary",
        "_RegisterStandardLibrary@4",
        "RegisterStandardLibrary@4"
    };

    for (const char* funcName : functionNames) {
        registerFunc = reinterpret_cast<TRegisterLibraryFunction>(
            GetProcAddress(FStandardLibraryHandle, funcName));
        if (registerFunc) break;
    }

    if (!registerFunc) {
        FreeLibrary(FStandardLibraryHandle);
        FStandardLibraryHandle = 0;
        return false;
    }

    try {
        bool result = registerFunc(FLibraryManager.get());
        if (!result) {
            FreeLibrary(FStandardLibraryHandle);
            FStandardLibraryHandle = 0;
            return false;
        }
        return true;
    }
    catch (...) {
        FreeLibrary(FStandardLibraryHandle);
        FStandardLibraryHandle = 0;
        return false;
    }
}

void __fastcall TMainForm::FormDestroy(TObject *Sender) {
    // Сначала очищаем элементы и соединения
    FElements.clear();
    FConnections.clear();
    FSelectedElements.clear();

    // Затем освобождаем библиотеки
    FLibraryManager.reset();

    // И только потом выгружаем DLL
    UnloadStandardLibrary();
}

void TMainForm::UnloadStandardLibrary() {
    if (FStandardLibraryHandle) {
        // Даем время на завершение операций
        Sleep(100);

        // Пытаемся найти функцию выгрузки
        TUnregisterLibraryFunction unregisterFunc =
            reinterpret_cast<TUnregisterLibraryFunction>(
                GetProcAddress(FStandardLibraryHandle, "UnregisterStandardLibrary"));

        if (unregisterFunc) {
            unregisterFunc(FLibraryManager.get());
        }

        FreeLibrary(FStandardLibraryHandle);
        FStandardLibraryHandle = 0;
    }
}

// Новые методы для создания элементов через библиотеки
// Обновленный метод CreateElement
std::unique_ptr<TCircuitElement> TMainForm::CreateElement(const String& LibraryName, const String& ElementName, int X, int Y) {
    try {
        if (!FLibraryManager) {
            throw Exception("Менеджер библиотек не инициализирован");
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


// Обновленный метод CreateElementFromCurrent с улучшенной обработкой ошибок
std::unique_ptr<TCircuitElement> TMainForm::CreateElementFromCurrent(const String& ElementName, int X, int Y) {
    try {
        if (!FLibraryManager) {
            throw Exception("Менеджер библиотек не инициализирован");
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

    // Устанавливаем новые границы
    newElement->SetBounds(TRect(bestPos.X, bestPos.Y, bestPos.X + width, bestPos.Y + height));
    newElement->CalculateRelativePositions();

    FElements.push_back(std::move(newElement));
    UpdatePaintBoxSize();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "Добавлен: " + elementName;
}

// Упрощенная сериализация для демонстрации
void TMainForm::SaveSchemeToFile(const String& FileName) {
    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    iniFile->WriteInteger("Scheme", "ElementCount", static_cast<int>(FElements.size()));
    iniFile->WriteInteger("Scheme", "ConnectionCount", static_cast<int>(FConnections.size()));
    iniFile->WriteInteger("Scheme", "NextElementId", FNextElementId);
    iniFile->WriteString("Scheme", "Version", "2.0");

    // Сохраняем информацию о библиотеках и элементах
    for (int i = 0; i < FElements.size(); i++) {
        String section = "Element_" + IntToStr(i);
        TCircuitElement* element = FElements[i].get();

        iniFile->WriteString(section, "Name", element->Name);
        iniFile->WriteInteger(section, "Id", element->Id);
        iniFile->WriteInteger(section, "X", element->Bounds.Left);
        iniFile->WriteInteger(section, "Y", element->Bounds.Top);
        iniFile->WriteInteger(section, "Width", element->Bounds.Width());
        iniFile->WriteInteger(section, "Height", element->Bounds.Height());
    }

    // Сохраняем соединения (упрощенно)
    for (int i = 0; i < FConnections.size(); i++) {
        String section = "Connection_" + IntToStr(i);
        auto& connection = FConnections[i];

        // Сохраняем индексы элементов и точки соединения
        if (connection.first && connection.second) {
            iniFile->WriteInteger(section, "FromElementId", connection.first->Owner->Id);
            iniFile->WriteInteger(section, "ToElementId", connection.second->Owner->Id);
            iniFile->WriteInteger(section, "FromPointIndex", 0); // Упрощенно
            iniFile->WriteInteger(section, "ToPointIndex", 0);   // Упрощенно
        }
    }

    iniFile->UpdateFile();
}

void TMainForm::LoadSchemeFromFile(const String& FileName) {
    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    // Проверяем версию формата
    String version = iniFile->ReadString("Scheme", "Version", "1.0");

    if (version == "1.0") {
        ShowMessage("Формат файла устарел. Используйте новую версию для сохранения схем.");
        return;
    }

    btnClearWorkspaceClick(nullptr);

    int elementCount = iniFile->ReadInteger("Scheme", "ElementCount", 0);
    int connectionCount = iniFile->ReadInteger("Scheme", "ConnectionCount", 0);
    FNextElementId = iniFile->ReadInteger("Scheme", "NextElementId", 1);

    // Находим границы загружаемой схемы для центрирования
    int minX = MAXINT, minY = MAXINT, maxX = 0, maxY = 0;
    std::vector<TRect> loadedBounds;

    // Сначала собираем информацию о всех элементах
    for (int i = 0; i < elementCount; i++) {
        String section = "Element_" + IntToStr(i);

        int x = iniFile->ReadInteger(section, "X", 0);
        int y = iniFile->ReadInteger(section, "Y", 0);
        int width = iniFile->ReadInteger(section, "Width", 80);
        int height = iniFile->ReadInteger(section, "Height", 60);

        TRect bounds(x, y, x + width, y + height);
        loadedBounds.push_back(bounds);

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x + width);
        maxY = std::max(maxY, y + height);
    }

    // Вычисляем смещение для центрирования
    int schemeWidth = maxX - minX;
    int schemeHeight = maxY - minY;

    // Центр видимой области
    TPoint visibleCenter = GetVisibleAreaCenter();

    // Смещение для центрирования схемы
    int offsetX = visibleCenter.X - (minX + schemeWidth / 2);
    int offsetY = visibleCenter.Y - (minY + schemeHeight / 2);

    // Загружаем элементы со смещением для центрирования
    for (int i = 0; i < elementCount; i++) {
        String section = "Element_" + IntToStr(i);

        String name = iniFile->ReadString(section, "Name", "");
        int id = iniFile->ReadInteger(section, "Id", 0);
        int x = iniFile->ReadInteger(section, "X", 0) + offsetX;
        int y = iniFile->ReadInteger(section, "Y", 0) + offsetY;
        int width = iniFile->ReadInteger(section, "Width", 80);
        int height = iniFile->ReadInteger(section, "Height", 60);

        // Создаем элемент по имени из файла
        auto element = CreateElementFromCurrent(name, x, y);
        if (element) {
            element->SetId(id);
            element->SetBounds(TRect(x, y, x + width, y + height));
            FElements.push_back(std::move(element));
        } else {
            ShowMessage("Не удалось создать элемент: " + name);
        }
    }

    // Обновляем размеры и центрируем
    UpdatePaintBoxSize();
    CenterCircuit();
    CircuitImage->Repaint();

    StatusBar->Panels->Items[0]->Text = "Схема загружена: " + FileName +
                                       " (элементов: " + IntToStr(elementCount) + ")";
}


void __fastcall TMainForm::miDeleteElementClick(TObject *Sender) {
    DeleteSelectedElements();
}

void __fastcall TMainForm::btnClearWorkspaceClick(TObject *Sender) {
    if (Application->MessageBox(L"Очистить всю рабочую область?", L"Подтверждение",
                               MB_YESNO | MB_ICONQUESTION) == ID_YES) {
        FElements.clear();
        FConnections.clear();
        FSelectedElements.clear();
        FSelectedElement = nullptr;
        FNextElementId = 1;

        UpdatePaintBoxSize();

        // Сбрасываем скролл в начало после очистки
        Workspace->HorzScrollBar->Position = 0;
        Workspace->VertScrollBar->Position = 0;

        CircuitImage->Repaint();
        StatusBar->Panels->Items[0]->Text = "Рабочая область очищена.";
    }
}


void __fastcall TMainForm::CircuitImagePaint(TObject *Sender) {
    DrawCircuit();
}

void TMainForm::DrawCircuit() {
    OptimizedDrawCircuit(CircuitImage->Canvas);
}

void TMainForm::OptimizedDrawCircuit(TCanvas* Canvas) {
    // Используем статический буфер для уменьшения создания/удаления объектов
    static std::unique_ptr<TBitmap> buffer;
    static int lastWidth = 0, lastHeight = 0;

    if (!buffer || lastWidth != CircuitImage->Width || lastHeight != CircuitImage->Height) {
        buffer = std::make_unique<TBitmap>();
        buffer->Width = CircuitImage->Width;
        buffer->Height = CircuitImage->Height;
        lastWidth = CircuitImage->Width;
        lastHeight = CircuitImage->Height;
    }

    TCanvas* canvas = buffer->Canvas;

    // Очистка фона
    canvas->Brush->Color = clWhite;
    canvas->FillRect(CircuitImage->ClientRect);

    // Статическая переменная для отслеживания изменения масштаба
    static double lastZoomFactor = 0;

    // Сетка рисуется только при изменении масштаба
    if (lastZoomFactor != FZoomFactor) {
        canvas->Pen->Color = clSilver;
        canvas->Pen->Style = psDot;
        int gridSize = static_cast<int>(20 * FZoomFactor);
        if (gridSize > 2) { // Не рисуем слишком мелкую сетку
            for (int x = 0; x < CircuitImage->Width; x += gridSize) {
                canvas->MoveTo(x, 0);
                canvas->LineTo(x, CircuitImage->Height);
            }
            for (int y = 0; y < CircuitImage->Height; y += gridSize) {
                canvas->MoveTo(0, y);
                canvas->LineTo(CircuitImage->Width, y);
            }
        }
        canvas->Pen->Style = psSolid;
        lastZoomFactor = FZoomFactor;
    }

    // Соединения
    canvas->Pen->Width = static_cast<int>(2 * FZoomFactor);
    for (auto& connection : FConnections) {
        TConnectionPoint* start = connection.first;
        TConnectionPoint* end = connection.second;

        canvas->Pen->Color = TernaryToColor(start->Value);

        TPoint screenStart = LogicalToScreen(TPoint(start->X, start->Y));
        TPoint screenEnd = LogicalToScreen(TPoint(end->X, end->Y));

        // Учитываем смещение скролла
        screenStart.Offset(-Workspace->HorzScrollBar->Position, -Workspace->VertScrollBar->Position);
        screenEnd.Offset(-Workspace->HorzScrollBar->Position, -Workspace->VertScrollBar->Position);

        canvas->MoveTo(screenStart.X, screenStart.Y);
        canvas->LineTo(screenEnd.X, screenEnd.Y);

        // Стрелка только для достаточно длинных линий
        int dx = screenEnd.X - screenStart.X;
        int dy = screenEnd.Y - screenStart.Y;
        double length = sqrt(dx*dx + dy*dy);
        if (length > 15) { // Минимальная длина для отображения стрелки
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
    canvas->Pen->Width = 1;

    // Элементы - рисуем в логических координатах с учетом масштаба и смещения
    for (auto& element : FElements) {
        // Сохраняем оригинальные bounds
        TRect originalBounds = element->Bounds;

        // Создаем временные bounds с учетом масштаба и смещения
        TRect screenBounds = LogicalToScreen(originalBounds);
        screenBounds.Offset(-Workspace->HorzScrollBar->Position, -Workspace->VertScrollBar->Position);

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
        screenBounds.Offset(-Workspace->HorzScrollBar->Position, -Workspace->VertScrollBar->Position);
        canvas->Rectangle(screenBounds);
        canvas->Pen->Style = psSolid;
        canvas->Pen->Width = 1;
        canvas->Brush->Style = bsSolid;
    }

    // Прямоугольник выделения (уже в экранных координатах)
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
        screenStart.Offset(-Workspace->HorzScrollBar->Position, -Workspace->VertScrollBar->Position);
        canvas->MoveTo(screenStart.X, screenStart.Y);
        TPoint mousePos = CircuitImage->ScreenToClient(Mouse->CursorPos);
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

void __fastcall TMainForm::WorkspaceMouseWheel(TObject *Sender, TShiftState Shift,
    int WheelDelta, TPoint &MousePos, bool &Handled) {

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

        Workspace->VertScrollBar->Position = Workspace->VertScrollBar->Position + scrollAmount;
        Workspace->HorzScrollBar->Position = Workspace->HorzScrollBar->Position + scrollAmount;

        CircuitImage->Repaint();
    }
}

void __fastcall TMainForm::CircuitImageMouseDown(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y) {

    // Преобразуем экранные координаты в логические
    TPoint logicalPos = ScreenToLogical(TPoint(
        X + Workspace->HorzScrollBar->Position,
        Y + Workspace->VertScrollBar->Position
    ));

    if (Button == mbLeft) {
        // ПЕРВОЕ: проверяем клик на точку соединения ВНЕ зависимости от текущего режима
        for (auto& element : FElements) {
            TConnectionPoint* conn = element->GetConnectionAt(logicalPos.X, logicalPos.Y);
            if (conn) {
                if (!FIsConnecting) {
                    // Начало нового соединения
                    if (!conn->IsInput) {
                        // Клик на выходной точке - начинаем соединение
                        FConnectionStart = conn;
                        FIsConnecting = true;
                        btnConnectionMode->Down = true; // Автоматически включаем режим соединения
                        StatusBar->Panels->Items[0]->Text = "Режим соединения. Выберите входную точку.";
                    } else {
                        StatusBar->Panels->Items[0]->Text = "Ошибка: первая точка должна быть выходом (синяя)";
                    }
                    CircuitImage->Repaint();
                    return;
                } else {
                    // Завершение существующего соединения
                    if (conn->IsInput && FConnectionStart && FConnectionStart->Owner != conn->Owner) {
                        bool connectionExists = false;
                        for (auto& existingConn : FConnections) {
                            if (existingConn.first == FConnectionStart && existingConn.second == conn) {
                                connectionExists = true;
                                break;
                            }
                        }

                        if (!connectionExists) {
                            FConnections.push_back(std::make_pair(FConnectionStart, conn));
                            StatusBar->Panels->Items[0]->Text = "Соединение создано.";
                        } else {
                            StatusBar->Panels->Items[0]->Text = "Соединение уже существует.";
                        }
                    } else {
                        StatusBar->Panels->Items[0]->Text = "Ошибка: вторая точка должна быть входом (зеленая) и принадлежать другому элементу";
                    }
                    FIsConnecting = false;
                    FConnectionStart = nullptr;
                    btnConnectionMode->Down = false; // Автоматически выходим из режима соединения
                    CircuitImage->Repaint();
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
            CircuitImage->Repaint();
            return;
        }

        // Стандартная обработка выделения и перемещения элементов
        FSelectedElement = nullptr;
        FDraggedElement = nullptr;

        if (!Shift.Contains(ssCtrl)) {
            FSelectedElements.clear();
        }

        bool elementFound = false;

        for (auto& element : FElements) {
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
            // FSelectionRect задается в экранных координатах
            FSelectionRect = TRect(X, Y, X, Y);
            if (!Shift.Contains(ssCtrl)) {
                FSelectedElements.clear();
            }
            StatusBar->Panels->Items[0]->Text = "Режим выделения: рисуйте прямоугольник";
        } else {
            StatusBar->Panels->Items[0]->Text = "Выбрано элементов: " + IntToStr(static_cast<int>(FSelectedElements.size()));
        }

        CircuitImage->Repaint();
    } else if (Button == mbRight) {
        FSelectedElement = nullptr;
        for (auto& element : FElements) {
            TRect bounds = element->Bounds;
            if (logicalPos.X >= bounds.Left && logicalPos.X <= bounds.Right &&
                logicalPos.Y >= bounds.Top && logicalPos.Y <= bounds.Bottom) {
                FSelectedElement = element.get();

                TPoint popupPos = CircuitImage->ClientToScreen(TPoint(X, Y));
                ElementPopupMenu->Popup(popupPos.X, popupPos.Y);
                break;
            }
        }
    }
}

void __fastcall TMainForm::CircuitImageMouseMove(TObject *Sender, TShiftState Shift, int X, int Y) {
    TPoint logicalPos = ScreenToLogical(TPoint(
        X + Workspace->HorzScrollBar->Position,
        Y + Workspace->VertScrollBar->Position
    ));

    // Оптимизация: обновляем только при значительном перемещении
    static int lastX = -1, lastY = -1;

    if (FIsDragging && FDraggedElement) {
        if (abs(logicalPos.X - lastX) > 1 || abs(logicalPos.Y - lastY) > 1) {
            int newLeft = logicalPos.X - FDragOffsetX;
            int newTop = logicalPos.Y - FDragOffsetY;

            TRect newBounds = TRect(
                newLeft,
                newTop,
                newLeft + FDraggedElement->Bounds.Width(),
                newTop + FDraggedElement->Bounds.Height()
            );

            FDraggedElement->SetBounds(newBounds);
            CircuitImage->Repaint();

            lastX = logicalPos.X;
            lastY = logicalPos.Y;
        }
    }
    else if (FIsSelecting) {
        FSelectionRect.Right = X;
        FSelectionRect.Bottom = Y;
        CircuitImage->Repaint();
    }

    // Обновление курсора
    if (btnConnectionMode->Down || FIsConnecting) {
        bool overConnection = false;
        for (auto& element : FElements) {
            if (element->GetConnectionAt(logicalPos.X, logicalPos.Y)) {
                overConnection = true;
                break;
            }
        }
        CircuitImage->Cursor = overConnection ? crHandPoint : crCross;
    } else {
        CircuitImage->Cursor = crDefault;
    }
}

void __fastcall TMainForm::CircuitImageMouseUp(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y) {
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
                normalizedRect.Left + Workspace->HorzScrollBar->Position,
                normalizedRect.Top + Workspace->VertScrollBar->Position
            ));
            TPoint logicalBottomRight = ScreenToLogical(TPoint(
                normalizedRect.Right + Workspace->HorzScrollBar->Position,
                normalizedRect.Bottom + Workspace->VertScrollBar->Position
            ));
            TRect logicalSelectionRect(logicalTopLeft, logicalBottomRight);

            for (auto& element : FElements) {
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
            CircuitImage->Repaint();
        } else {
            FIsDragging = false;
            FDraggedElement = nullptr;
        }
    }
}

void __fastcall TMainForm::btnRunSimulationClick(TObject *Sender) {
    RunSimulationStep();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "Симуляция выполнена.";
}

void TMainForm::RunSimulationStep() {
    for (auto& connection : FConnections) {
        if (connection.first && connection.second) {
            connection.second->Value = connection.first->Value;
        }
    }

    for (auto& element : FElements) {
        element->Calculate();
    }
}

void __fastcall TMainForm::btnResetSimulationClick(TObject *Sender) {
    ResetSimulation();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "Симуляция сброшена.";
}

void TMainForm::ResetSimulation() {
    for (auto& element : FElements) {
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

    for (auto& connection : FConnections) {
        if (connection.first) connection.first->Value = TTernary::ZERO;
        if (connection.second) connection.second->Value = TTernary::ZERO;
    }
}

void __fastcall TMainForm::btnConnectionModeClick(TObject *Sender) {
    if (btnConnectionMode->Down) {
        StatusBar->Panels->Items[0]->Text = "Режим соединения: выбирайте точки для соединения элементов.";
        CircuitImage->Cursor = crCross;
    } else {
        StatusBar->Panels->Items[0]->Text = "Режим выбора: выбирайте и перемещайте элементы.";
        CircuitImage->Cursor = crDefault;
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
    if (FSelectedElement) {
        TRect newBounds = FSelectedElement->Bounds;
        int width = newBounds.Width();
        int height = newBounds.Height();

        newBounds.Right = newBounds.Left + height;
        newBounds.Bottom = newBounds.Top + width;

        FSelectedElement->SetBounds(newBounds);
        UpdatePaintBoxSize();
        CircuitImage->Repaint();
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
    CircuitImage->Repaint();
}

void __fastcall TMainForm::FormResize(TObject *Sender) {
    UpdatePaintBoxSize();
    CircuitImage->Repaint();
}

void __fastcall TMainForm::WorkspaceResize(TObject *Sender) {
    UpdatePaintBoxSize();
    CircuitImage->Repaint();
}

void TMainForm::UpdatePaintBoxSize() {
    TRect bounds = GetCircuitBounds();

    int padding = 400;
    int newWidth = std::max(Workspace->ClientWidth, bounds.Width() + padding);
    int newHeight = std::max(Workspace->ClientHeight, bounds.Height() + padding);

    CircuitImage->Width = static_cast<int>(newWidth * FZoomFactor);
    CircuitImage->Height = static_cast<int>(newHeight * FZoomFactor);
}

void TMainForm::CenterCircuit() {
    if (FElements.empty()) return;

    TRect bounds = GetCircuitBounds();

    // Вычисляем центр схемы
    int centerX = (bounds.Left + bounds.Right) / 2;
    int centerY = (bounds.Top + bounds.Bottom) / 2;

    // Вычисляем центр видимой области в логических координатах
    int visibleWidth = Workspace->ClientWidth / FZoomFactor;
    int visibleHeight = Workspace->ClientHeight / FZoomFactor;

    int visibleCenterX = Workspace->HorzScrollBar->Position / FZoomFactor + visibleWidth / 2;
    int visibleCenterY = Workspace->VertScrollBar->Position / FZoomFactor + visibleHeight / 2;

    // Вычисляем новое положение для центрирования
    int newScrollX = (centerX - visibleWidth / 2) * FZoomFactor;
    int newScrollY = (centerY - visibleHeight / 2) * FZoomFactor;

    // Ограничиваем значения скролла
    newScrollX = std::max(0, newScrollX);
    newScrollY = std::max(0, newScrollY);
    newScrollX = std::min(newScrollX, Workspace->HorzScrollBar->Range - visibleWidth);
    newScrollY = std::min(newScrollY, Workspace->VertScrollBar->Range - visibleHeight);

    Workspace->HorzScrollBar->Position = newScrollX;
    Workspace->VertScrollBar->Position = newScrollY;

    CircuitImage->Repaint();
}


TRect TMainForm::GetCircuitBounds() {
    if (FElements.empty()) {
        return TRect(0, 0, CircuitImage->Width, CircuitImage->Height);
    }

    TRect bounds = FElements[0]->Bounds;
    for (auto& element : FElements) {
        bounds.Left = std::min(bounds.Left, element->Bounds.Left);
        bounds.Top = std::min(bounds.Top, element->Bounds.Top);
        bounds.Right = std::max(bounds.Right, element->Bounds.Right);
        bounds.Bottom = std::max(bounds.Bottom, element->Bounds.Bottom);
    }

    return bounds;
}

void __fastcall TMainForm::btnSaveSchemeClick(TObject *Sender) {
    if (SaveDialog->Execute()) {
        try {
            SaveSchemeToFile(SaveDialog->FileName);
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
                LoadSchemeFromFile(OpenDialog->FileName);
                StatusBar->Panels->Items[0]->Text = "Схема загружена: " + OpenDialog->FileName;
            }
        }
        catch (Exception &e) {
            Application->MessageBox(L"Ошибка при загрузке схемы", L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }
}

std::vector<TCircuitElement*> TMainForm::GetSelectedElements() {
    return FSelectedElements;
}


void TMainForm::CreateSubCircuitFromSelection() {
    auto selectedElements = GetSelectedElements();

    if (selectedElements.size() < 2) {
        StatusBar->Panels->Items[0]->Text = "Выберите хотя бы 2 элемента для группировки";
        return;
    }

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
    for (auto& connection : FConnections) {
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
        auto it = std::find_if(FElements.begin(), FElements.end(),
            [selectedElement](const std::unique_ptr<TCircuitElement>& elem) {
                return elem.get() == selectedElement;
            });

        if (it != FElements.end()) {
            subCircuitElements.push_back(std::move(*it));
        }
    }

    // Удаляем элементы из основного списка
    FElements.erase(std::remove_if(FElements.begin(), FElements.end(),
        [](const std::unique_ptr<TCircuitElement>& elem) {
            return elem == nullptr;
        }), FElements.end());

    // Удаляем внутренние соединения из основного списка
    for (auto& internalConn : internalConnections) {
        auto it = std::find(FConnections.begin(), FConnections.end(), internalConn);
        if (it != FConnections.end()) {
            FConnections.erase(it);
        }
    }

    // Создаем подсхему
    auto subCircuit = std::make_unique<TSubCircuit>(FNextElementId++, centerX, centerY,
                                                   std::move(subCircuitElements),
                                                   internalConnections);

    FSelectedElement = subCircuit.get();
    FSelectedElements.clear();
    FSelectedElements.push_back(subCircuit.get());

    FElements.push_back(std::move(subCircuit));

    UpdatePaintBoxSize();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "Создана подсхема из " + IntToStr(static_cast<int>(selectedElements.size())) + " элементов";
}

TConnectionPoint* TMainForm::FindRestoredConnectionPoint(const TConnectionPoint* originalPoint) {
    if (!originalPoint || !originalPoint->Owner) return nullptr;

    // Ищем восстановленный элемент по оригинальным координатам и имени
    for (auto& element : FElements) {
        if (element->Bounds.Left == originalPoint->Owner->Bounds.Left &&
            element->Bounds.Top == originalPoint->Owner->Bounds.Top &&
            element->Name == originalPoint->Owner->Name) {

            // Находим соответствующую точку соединения по относительным координатам
            if (originalPoint->IsInput) {
                for (auto& input : element->Inputs) {
                    // Сравниваем относительные позиции для точного соответствия
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

    const auto& internalElements = subCircuit->GetInternalElements();
    const auto& internalConnections = subCircuit->GetInternalConnections();

    // Восстанавливаем элементы в основной схеме
    for (const auto& element : internalElements) {
        // Восстанавливаем оригинальные позиции элементов
        TRect originalBounds = element->Bounds;

        // Создаем новый элемент того же типа
        auto newElement = CreateElementFromCurrent(element->Name, originalBounds.Left, originalBounds.Top);
        if (newElement) {
            newElement->SetBounds(originalBounds);
            newElement->CalculateRelativePositions(); // Важно: пересчитываем позиции точек

            // Восстанавливаем состояния
            for (size_t i = 0; i < std::min(element->Inputs.size(), newElement->Inputs.size()); ++i) {
                newElement->Inputs[i].Value = element->Inputs[i].Value;
                // Копируем относительные позиции для точного восстановления
                newElement->Inputs[i].RelX = element->Inputs[i].RelX;
                newElement->Inputs[i].RelY = element->Inputs[i].RelY;
            }
            for (size_t i = 0; i < std::min(element->Outputs.size(), newElement->Outputs.size()); ++i) {
                newElement->Outputs[i].Value = element->Outputs[i].Value;
                newElement->Outputs[i].RelX = element->Outputs[i].RelX;
                newElement->Outputs[i].RelY = element->Outputs[i].RelY;
            }

            FElements.push_back(std::move(newElement));
        }
    }

    // Восстанавливаем ВСЕ соединения
    for (const auto& conn : internalConnections) {
        // Находим соответствующие точки в восстановленных элементах
        TConnectionPoint* fromPoint = FindRestoredConnectionPoint(conn.first);
        TConnectionPoint* toPoint = FindRestoredConnectionPoint(conn.second);

        if (fromPoint && toPoint) {
            FConnections.push_back(std::make_pair(fromPoint, toPoint));
        }
    }

    // Удаляем подсхему
    auto it = std::find_if(FElements.begin(), FElements.end(),
        [SubCircuit](const std::unique_ptr<TCircuitElement>& elem) {
            return elem.get() == SubCircuit;
        });

    if (it != FElements.end()) {
        FElements.erase(it);
    }

    FSelectedElement = nullptr;
    FSelectedElements.clear();

    UpdatePaintBoxSize();
    CircuitImage->Repaint();
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
