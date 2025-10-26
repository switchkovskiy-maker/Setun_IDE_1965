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

// Реализация TSubCircuit с умными указателями
TSubCircuit::TSubCircuit(int AId, int X, int Y,
                        std::vector<std::unique_ptr<TCircuitElement>> Elements,
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

    int inputY = FBounds.Top + 20;
    int outputY = FBounds.Top + 30;

    for (auto& element : FInternalElements) {
        for (const auto& input : element->Inputs) {
            bool isExternal = true;
            for (const auto& conn : FInternalConnections) {
                if (conn.second == &input) {
                    isExternal = false;
                    break;
                }
            }
            if (isExternal) {
                FInputs.push_back(TConnectionPoint(this, FBounds.Left - 15, inputY,
                    TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
                inputY += 20;
                if (inputY > FBounds.Bottom - 20) inputY = FBounds.Top + 20;
            }
        }

        for (const auto& output : element->Outputs) {
            bool isExternal = true;
            for (const auto& conn : FInternalConnections) {
                if (conn.first == &output) {
                    isExternal = false;
                    break;
                }
            }
            if (isExternal) {
                FOutputs.push_back(TConnectionPoint(this, FBounds.Right + 15, outputY,
                    TTernary::ZERO, false, TLineStyle::OUTPUT_LINE));
                outputY += 20;
                if (outputY > FBounds.Bottom - 20) outputY = FBounds.Top + 30;
            }
        }
    }

    if (FInputs.empty()) {
        FInputs.push_back(TConnectionPoint(this, FBounds.Left - 15, FBounds.Top + 20,
            TTernary::ZERO, true, TLineStyle::POSITIVE_CONTROL));
    }
    if (FOutputs.empty()) {
        FOutputs.push_back(TConnectionPoint(this, FBounds.Right + 15, FBounds.Top + 30,
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

    // Регистрируем базовые элементы
    FBasicLibrary->RegisterElement<TMagneticAmplifier>("Магнитный усилитель", "Простой магнитный усилитель", "Усилители");
    FBasicLibrary->RegisterElement<TTernaryElement>("Троичный элемент", "Базовый троичный элемент", "Логика");
    FBasicLibrary->RegisterElement<TShiftRegister>("Сдвигающий регистр", "4-битный сдвигающий регистр", "Память", 100, 60);

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
    // Очищаем элементы - теперь они автоматически удаляются через умные указатели
    FElements.clear();

    // Очищаем соединения
    FConnections.clear();
    FSelectedElements.clear();

    UnloadStandardLibrary();
    FLibraryManager.reset();
}

void TMainForm::UnloadStandardLibrary() {
    if (FStandardLibraryHandle) {
        FreeLibrary(FStandardLibraryHandle);
        FStandardLibraryHandle = 0;
    }
}

// Новые методы для создания элементов через библиотеки
std::unique_ptr<TCircuitElement> TMainForm::CreateElement(const String& LibraryName, const String& ElementName, int X, int Y) {
    try {
        auto element = FLibraryManager->CreateElement(LibraryName, ElementName, FNextElementId, X, Y);
        if (element) {
            element->CalculateRelativePositions();
            FNextElementId++;
        }
        return element;
    }
    catch (Exception &e) {
        ShowMessage("Ошибка создания элемента '" + ElementName + "' из библиотеки '" + LibraryName + "': " + e.Message);
        return nullptr;
    }
}

std::unique_ptr<TCircuitElement> TMainForm::CreateElementFromCurrent(const String& ElementName, int X, int Y) {
    try {
        auto element = FLibraryManager->CreateElementFromCurrent(ElementName, FNextElementId, X, Y);
        if (element) {
            element->CalculateRelativePositions();
            FNextElementId++;
        }
        return element;
    }
    catch (Exception &e) {
        ShowMessage("Ошибка создания элемента '" + ElementName + "': " + e.Message);
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
    if (ElementLibrary->ItemIndex < 0) {
        return;
    }

    String elementName = ElementLibrary->Items->Strings[ElementLibrary->ItemIndex];

    int x, y;
    TPoint mousePos = CircuitImage->ScreenToClient(Mouse->CursorPos);
    if (mousePos.X > 0 && mousePos.Y > 0 &&
        mousePos.X < CircuitImage->Width && mousePos.Y < CircuitImage->Height) {
        x = mousePos.X - 40;
        y = mousePos.Y - 30;
    } else {
        x = CircuitImage->Width / 2 - 40;
        y = CircuitImage->Height / 2 - 30;
    }

    auto newElement = CreateElementFromCurrent(elementName, x, y);
    if (newElement) {
        FElements.push_back(std::move(newElement));
        UpdatePaintBoxSize();
        CircuitImage->Repaint();
        StatusBar->Panels->Items[0]->Text = "Добавлен: " + elementName;
    } else {
        StatusBar->Panels->Items[0]->Text = "Ошибка создания элемента: " + elementName;
    }
}

// Упрощенная сериализация для демонстрации
void TMainForm::SaveSchemeToFile(const String& FileName) {
    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    iniFile->WriteInteger("Scheme", "ElementCount", static_cast<int>(FElements.size()));
    iniFile->WriteInteger("Scheme", "ConnectionCount", static_cast<int>(FConnections.size()));
    iniFile->WriteInteger("Scheme", "NextElementId", FNextElementId);
    iniFile->WriteString("Scheme", "Version", "2.0"); // Новая версия формата

    // Сохраняем информацию о библиотеках и элементах
    for (int i = 0; i < FElements.size(); i++) {
        String section = "Element_" + IntToStr(i);
        TCircuitElement* element = FElements[i].get();

        // Для упрощения сохраняем только базовую информацию
        // В реальной реализации нужно сохранять библиотеку и имя элемента
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

        // Сохраняем индексы элементов и точек соединения
        // В реальной реализации нужна более сложная логика
    }

    iniFile->UpdateFile();
}

void TMainForm::LoadSchemeFromFile(const String& FileName) {
    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    // Проверяем версию формата
    String version = iniFile->ReadString("Scheme", "Version", "1.0");

    if (version == "1.0") {
        // Загрузка старого формата (для обратной совместимости)
        // В реальной реализации нужно добавить конвертацию
        ShowMessage("Формат файла устарел. Используйте новую версию для сохранения схем.");
        return;
    }

    btnClearWorkspaceClick(nullptr);

    int elementCount = iniFile->ReadInteger("Scheme", "ElementCount", 0);
    int connectionCount = iniFile->ReadInteger("Scheme", "ConnectionCount", 0);
    FNextElementId = iniFile->ReadInteger("Scheme", "NextElementId", 1);

    // Упрощенная загрузка - создаем элементы по имени
    for (int i = 0; i < elementCount; i++) {
        String section = "Element_" + IntToStr(i);

        String name = iniFile->ReadString(section, "Name", "");
        int id = iniFile->ReadInteger(section, "Id", 0);
        int x = iniFile->ReadInteger(section, "X", 0);
        int y = iniFile->ReadInteger(section, "Y", 0);
        int width = iniFile->ReadInteger(section, "Width", 80);
        int height = iniFile->ReadInteger(section, "Height", 60);

        // Пытаемся создать элемент по имени из текущей библиотеки
        auto element = CreateElementFromCurrent(name, x, y);
        if (element) {
            element->SetId(id);
            element->SetBounds(TRect(x, y, x + width, y + height));
            FElements.push_back(std::move(element));
        }
    }

    // Восстанавливаем соединения (упрощенно)
    // В реальной реализации нужна более сложная логика

    UpdatePaintBoxSize();
    CircuitImage->Repaint();
}

// Остальные методы остаются в основном без изменений, но адаптированы под умные указатели
void __fastcall TMainForm::miDeleteElementClick(TObject *Sender) {
    if (FSelectedElement) {
        // Удаляем соединения связанные с элементом
        auto it = FConnections.begin();
        while (it != FConnections.end()) {
            if ((it->first->Owner == FSelectedElement) || (it->second->Owner == FSelectedElement)) {
                it = FConnections.erase(it);
            } else {
                ++it;
            }
        }

        // Удаляем элемент из вектора
        auto elemIt = std::find_if(FElements.begin(), FElements.end(),
            [this](const std::unique_ptr<TCircuitElement>& elem) {
                return elem.get() == FSelectedElement;
            });

        if (elemIt != FElements.end()) {
            FElements.erase(elemIt);
        }

        // Удаляем из выделенных элементов
        auto selIt = std::find(FSelectedElements.begin(), FSelectedElements.end(), FSelectedElement);
        if (selIt != FSelectedElements.end()) {
            FSelectedElements.erase(selIt);
        }

        FSelectedElement = nullptr;
        UpdatePaintBoxSize();
        CircuitImage->Repaint();
        StatusBar->Panels->Items[0]->Text = "Элемент удален.";
    }
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
        CircuitImage->Repaint();
        StatusBar->Panels->Items[0]->Text = "Рабочая область очищена.";
    }
}

// Остальные методы (DrawCircuit, обработчики мыши и т.д.) остаются в основном без изменений
// но адаптируются для работы с новой архитектурой
// Продолжение MainForm.cpp

void __fastcall TMainForm::CircuitImagePaint(TObject *Sender) {
    DrawCircuit();
}

void TMainForm::DrawCircuit() {
    std::unique_ptr<TBitmap> buffer(new TBitmap);
    buffer->Width = CircuitImage->Width;
    buffer->Height = CircuitImage->Height;

    TCanvas* canvas = buffer->Canvas;

    canvas->Brush->Color = clWhite;
    canvas->FillRect(CircuitImage->ClientRect);

    // Сетка
    canvas->Pen->Color = clSilver;
    canvas->Pen->Style = psDot;
    for (int x = 0; x < CircuitImage->Width; x += 20) {
        canvas->MoveTo(x, 0);
        canvas->LineTo(x, CircuitImage->Height);
    }
    for (int y = 0; y < CircuitImage->Height; y += 20) {
        canvas->MoveTo(0, y);
        canvas->LineTo(CircuitImage->Width, y);
    }
    canvas->Pen->Style = psSolid;

    // Соединения
    canvas->Pen->Width = 2;
    for (auto& connection : FConnections) {
        TConnectionPoint* start = connection.first;
        TConnectionPoint* end = connection.second;

        canvas->Pen->Color = TernaryToColor(start->Value);

        canvas->MoveTo(start->X, start->Y);
        canvas->LineTo(end->X, end->Y);

        int dx = end->X - start->X;
        int dy = end->Y - start->Y;
        double length = sqrt(dx*dx + dy*dy);
        if (length > 0) {
            double unitX = dx / length;
            double unitY = dy / length;

            int arrowSize = 6;
            int arrowX = end->X - (int)(unitX * arrowSize);
            int arrowY = end->Y - (int)(unitY * arrowSize);

            canvas->MoveTo(arrowX - (int)(unitY * arrowSize/2), arrowY + (int)(unitX * arrowSize/2));
            canvas->LineTo(end->X, end->Y);
            canvas->LineTo(arrowX + (int)(unitY * arrowSize/2), arrowY - (int)(unitX * arrowSize/2));
        }
    }
    canvas->Pen->Width = 1;

    // Элементы
    for (auto& element : FElements) {
        element->Draw(canvas);
    }

    // Выделенные элементы
    for (auto selectedElement : FSelectedElements) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Width = 2;
        canvas->Pen->Style = psDash;
        canvas->Brush->Style = bsClear;
        canvas->Rectangle(selectedElement->Bounds);
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
        canvas->MoveTo(FConnectionStart->X, FConnectionStart->Y);
        TPoint mousePos = CircuitImage->ScreenToClient(Mouse->CursorPos);
        canvas->LineTo(mousePos.X, mousePos.Y);
        canvas->Pen->Style = psSolid;
    }

    CircuitImage->Canvas->Draw(0, 0, buffer.get());
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
    }
}

void __fastcall TMainForm::CircuitImageMouseDown(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y) {

    if (Button == mbLeft) {
        if (btnConnectionMode->Down) {
            for (auto& element : FElements) {
                TConnectionPoint* conn = element->GetConnectionAt(X, Y);
                if (conn) {
                    if (!FIsConnecting) {
                        if (!conn->IsInput) {
                            FConnectionStart = conn;
                            FIsConnecting = true;
                            StatusBar->Panels->Items[0]->Text = "Выбрана выходная точка. Выберите входную точку.";
                        } else {
                            StatusBar->Panels->Items[0]->Text = "Ошибка: первая точка должна быть выходом (синяя)";
                        }
                        CircuitImage->Repaint();
                    } else {
                        if (conn->IsInput) {
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
                            StatusBar->Panels->Items[0]->Text = "Ошибка: вторая точка должна быть входом (зеленая)";
                        }
                        FIsConnecting = false;
                        FConnectionStart = nullptr;
                        CircuitImage->Repaint();
                    }
                    return;
                }
            }

            if (FIsConnecting) {
                FIsConnecting = false;
                FConnectionStart = nullptr;
                StatusBar->Panels->Items[0]->Text = "Соединение отменено.";
                CircuitImage->Repaint();
            }
        }
        else {
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

                if (X >= expandedBounds.Left && X <= expandedBounds.Right &&
                    Y >= expandedBounds.Top && Y <= expandedBounds.Bottom) {

                    FSelectedElement = element.get();
                    elementFound = true;

                    auto it = std::find(FSelectedElements.begin(), FSelectedElements.end(), element.get());
                    if (it == FSelectedElements.end()) {
                        FSelectedElements.push_back(element.get());
                    }

                    FDraggedElement = element.get();
                    FIsDragging = true;

                    FDragOffsetX = X - bounds.Left;
                    FDragOffsetY = Y - bounds.Top;

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

            CircuitImage->Repaint();
        }
    } else if (Button == mbRight) {
        FSelectedElement = nullptr;
        for (auto& element : FElements) {
            if (X >= element->Bounds.Left && X <= element->Bounds.Right &&
                Y >= element->Bounds.Top && Y <= element->Bounds.Bottom) {
                FSelectedElement = element.get();

                TPoint popupPos = CircuitImage->ClientToScreen(TPoint(X, Y));
                ElementPopupMenu->Popup(popupPos.X, popupPos.Y);
                break;
            }
        }
    }
}

void __fastcall TMainForm::CircuitImageMouseMove(TObject *Sender, TShiftState Shift, int X, int Y) {
    if (FIsDragging && FDraggedElement) {
        int newLeft = X - FDragOffsetX;
        int newTop = Y - FDragOffsetY;

        TRect newBounds = TRect(
            newLeft,
            newTop,
            newLeft + FDraggedElement->Bounds.Width(),
            newTop + FDraggedElement->Bounds.Height()
        );

        FDraggedElement->SetBounds(newBounds);

        UpdatePaintBoxSize();
        CircuitImage->Repaint();
    }
    else if (FIsSelecting) {
        FSelectionRect.Right = X;
        FSelectionRect.Bottom = Y;
        CircuitImage->Repaint();
    }

    if (btnConnectionMode->Down) {
        bool overConnection = false;
        for (auto& element : FElements) {
            if (element->GetConnectionAt(X, Y)) {
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

            for (auto& element : FElements) {
                TRect bounds = element->Bounds;
                if (bounds.Left <= normalizedRect.Right && bounds.Right >= normalizedRect.Left &&
                    bounds.Top <= normalizedRect.Bottom && bounds.Bottom >= normalizedRect.Top) {

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
    for (auto& element : FElements) {
        TRect bounds = element->Bounds;
        bounds.Left = static_cast<int>(bounds.Left * FZoomFactor);
        bounds.Top = static_cast<int>(bounds.Top * FZoomFactor);
        bounds.Right = static_cast<int>(bounds.Right * FZoomFactor);
        bounds.Bottom = static_cast<int>(bounds.Bottom * FZoomFactor);
        element->SetBounds(bounds);
        element->CalculateRelativePositions();
    }

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
    FScrollOffsetX = (CircuitImage->Width - bounds.Width()) / 2 - bounds.Left;
    FScrollOffsetY = (CircuitImage->Height - bounds.Height()) / 2 - bounds.Top;
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
                                                   std::move(subCircuitElements), internalConnections);

    FSelectedElement = subCircuit.get();
    FSelectedElements.clear();
    FSelectedElements.push_back(subCircuit.get());

    FElements.push_back(std::move(subCircuit));

    UpdatePaintBoxSize();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "Создана подсхема из " + IntToStr(static_cast<int>(selectedElements.size())) + " элементов";
}

void TMainForm::UngroupSubCircuit(TCircuitElement* SubCircuit) {
    TSubCircuit* subCircuit = dynamic_cast<TSubCircuit*>(SubCircuit);
    if (!subCircuit) return;

    const auto& internalElements = subCircuit->GetInternalElements();
    const auto& internalConnections = subCircuit->GetInternalConnections();

    // Упрощенная реализация - в реальном коде нужно клонировать элементы
    // Временно отключаем сложную логику разгруппировки
    StatusBar->Panels->Items[0]->Text = "Разгруппировка временно отключена. Используйте пересоздание элементов.";

    // Просто удаляем подсхему
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
    StatusBar->Panels->Items[0]->Text = "Подсхема удалена (разгруппировка временно отключена)";
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
