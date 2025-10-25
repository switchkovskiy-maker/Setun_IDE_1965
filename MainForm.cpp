#include "MainForm.h"
#include "CircuitElement.h"
#include "CircuitElements.h"
#include "ComponentLibrary.h"
#include "StandardLibrary.h"
#include <Vcl.Dialogs.hpp>
#include <Vcl.Graphics.hpp>
#include <math.h>
#include <algorithm>
#include <memory>
#include <System.IOUtils.hpp>

#pragma package(smart_init)
#pragma resource "*.dfm"

TMainForm *MainForm;

// Реализация методов TSubCircuit
TSubCircuit::TSubCircuit(int AId, int X, int Y, const std::vector<TCircuitElement*>& Elements,
            const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& Connections)
    : TCircuitElement(AId, "SubCircuit", TElementType::SUBCIRCUIT, X, Y) {

    FBounds = TRect(X, Y, X + 120, Y + 80);
    FInternalElements = Elements;
    FInternalConnections = Connections;

    CreateExternalConnections();
    CalculateRelativePositions();
}

void TSubCircuit::Calculate() {
    for (auto element : FInternalElements) {
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

    for (auto element : FInternalElements) {
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
    FIsConnecting(false), FIsDragging(false), FIsMultiSelecting(false),
    FNextElementId(1), FDragOffsetX(0), FDragOffsetY(0), FZoomFactor(1.0),
    FScrollOffsetX(0), FScrollOffsetY(0) {
}

void __fastcall TMainForm::FormCreate(TObject *Sender) {
    // Инициализируем менеджер библиотек
    LibraryManager = std::make_unique<TLibraryManager>();
    RegisterStandardLibrary();
    
    CreateCompleteLibrary();
    CircuitImage->Align = alNone;
    CircuitImage->Cursor = crCross;
    UpdatePaintBoxSize();
    CenterCircuit();

    btnConnectionMode->AllowAllUp = true;
    btnConnectionMode->GroupIndex = 1;
    btnMultiSelect->AllowAllUp = true;
    btnMultiSelect->GroupIndex = 2;

    // Инициализируем селектор библиотек
    UpdateLibrarySelector();
    
    StatusBar->Panels->Items[0]->Text = "Готов к работе. Выберите элемент из библиотеки или режим соединения.";
    Caption = "Setun IDE - Троичная логика по книге 1965 года";
}

void __fastcall TMainForm::FormDestroy(TObject *Sender) {
    for (auto element : FElements) {
        delete element;
    }
    LibraryManager.reset(); // Освобождаем менеджер библиотек
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
    for (auto element : FElements) {
        bounds.Left = std::min(bounds.Left, element->Bounds.Left);
        bounds.Top = std::min(bounds.Top, element->Bounds.Top);
        bounds.Right = std::max(bounds.Right, element->Bounds.Right);
        bounds.Bottom = std::max(bounds.Bottom, element->Bounds.Bottom);
    }

    return bounds;
}

void TMainForm::UpdateLibrarySelector() {
    cmbLibrarySelector->Items->Clear();
    auto libraryNames = LibraryManager->GetLibraryNames();
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
    
    auto currentLib = LibraryManager->GetCurrentLibrary();
    if (currentLib) {
        auto elements = currentLib->ElementDescriptions;
        for (const auto& element : elements) {
            ElementLibrary->Items->Add(element);
        }
        
        LibraryLabel->Caption = "Библиотека: " + currentLib->Name;
        StatusBar->Panels->Items[0]->Text = "Загружена библиотека: " + currentLib->Name;
    }
}

void __fastcall TMainForm::cmbLibrarySelectorChange(TObject *Sender) {
    if (cmbLibrarySelector->ItemIndex >= 0) {
        String libraryName = cmbLibrarySelector->Items->Strings[cmbLibrarySelector->ItemIndex];
        LibraryManager->SetCurrentLibrary(libraryName);
        LoadCurrentLibrary();
    }
}

void TMainForm::CreateCompleteLibrary() {
    // Теперь библиотека загружается через LibraryManager
    UpdateLibrarySelector();
}

TCircuitElement* TMainForm::CreateElementByType(const String& ElementDesc, int X, int Y) {
    TCircuitElement* element = LibraryManager->CreateElementFromCurrent(ElementDesc, FNextElementId++, X, Y);
    if (element) {
        element->CalculateRelativePositions();
    }
    return element;
}

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

    for (auto element : FElements) {
        element->Draw(canvas);
    }

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

    if (FIsMultiSelecting && FIsDragging) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Style = psDash;
        canvas->Brush->Style = bsClear;
        canvas->Rectangle(FSelectionRect);
        canvas->Pen->Style = psSolid;
        canvas->Brush->Style = bsSolid;
    }

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

void __fastcall TMainForm::btnMultiSelectClick(TObject *Sender) {
    FIsMultiSelecting = btnMultiSelect->Down;
    if (FIsMultiSelecting) {
        StatusBar->Panels->Items[0]->Text = "Режим множественного выделения: рисуйте прямоугольник для выбора элементов";
        CircuitImage->Cursor = crCross;
    } else {
        StatusBar->Panels->Items[0]->Text = "Режим одиночного выделения";
        CircuitImage->Cursor = crDefault;
    }
}

void __fastcall TMainForm::CircuitImageMouseDown(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y) {

    if (Button == mbLeft) {
        if (btnConnectionMode->Down) {
            for (auto element : FElements) {
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
        else if (FIsMultiSelecting) {
            FSelectionRect = TRect(X, Y, X, Y);
            FIsDragging = true;
            StatusBar->Panels->Items[0]->Text = "Выделение: рисуйте прямоугольник";
        }
        else {
            FSelectedElement = nullptr;
            FDraggedElement = nullptr;

            if (!Shift.Contains(ssCtrl)) {
                FSelectedElements.clear();
            }

            bool elementFound = false;
            for (auto element : FElements) {
                TRect bounds = element->Bounds;
                TRect expandedBounds = TRect(
                    bounds.Left - 5, bounds.Top - 5,
                    bounds.Right + 5, bounds.Bottom + 5
                );

                if (X >= expandedBounds.Left && X <= expandedBounds.Right &&
                    Y >= expandedBounds.Top && Y <= expandedBounds.Bottom) {

                    FSelectedElement = element;
                    elementFound = true;

                    auto it = std::find(FSelectedElements.begin(), FSelectedElements.end(), element);
                    if (it == FSelectedElements.end()) {
                        FSelectedElements.push_back(element);
                    }

                    FDraggedElement = element;
                    FIsDragging = true;

                    FDragOffsetX = X - bounds.Left;
                    FDragOffsetY = Y - bounds.Top;

                    StatusBar->Panels->Items[0]->Text = "Выбран элемент: " + element->Name;
                    break;
                }
            }

            if (!elementFound) {
                if (!Shift.Contains(ssCtrl)) {
                    FSelectedElements.clear();
                }
                StatusBar->Panels->Items[0]->Text = "Ничего не выбрано.";
            } else {
                StatusBar->Panels->Items[0]->Text = "Выбрано элементов: " + IntToStr(static_cast<int>(FSelectedElements.size()));
            }

            CircuitImage->Repaint();
        }
    } else if (Button == mbRight) {
        FSelectedElement = nullptr;
        for (auto element : FElements) {
            if (X >= element->Bounds.Left && X <= element->Bounds.Right &&
                Y >= element->Bounds.Top && Y <= element->Bounds.Bottom) {
                FSelectedElement = element;

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
    else if (FIsMultiSelecting && FIsDragging) {
        FSelectionRect.Right = X;
        FSelectionRect.Bottom = Y;
        CircuitImage->Repaint();
    }

    if (btnConnectionMode->Down) {
        bool overConnection = false;
        for (auto element : FElements) {
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
        if (FIsMultiSelecting && FIsDragging) {
            FIsDragging = false;

            TRect normalizedRect = FSelectionRect;
            if (normalizedRect.Left > normalizedRect.Right) {
                std::swap(normalizedRect.Left, normalizedRect.Right);
            }
            if (normalizedRect.Top > normalizedRect.Bottom) {
                std::swap(normalizedRect.Top, normalizedRect.Bottom);
            }

            FSelectedElements.clear();
            FSelectedElement = nullptr;

            for (auto element : FElements) {
                TRect bounds = element->Bounds;
                if (bounds.Left >= normalizedRect.Left && bounds.Right <= normalizedRect.Right &&
                    bounds.Top >= normalizedRect.Top && bounds.Bottom <= normalizedRect.Bottom) {

                    FSelectedElements.push_back(element);
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

void __fastcall TMainForm::ElementLibraryDblClick(TObject *Sender) {
    if (ElementLibrary->ItemIndex >= 0) {
        String elementDesc = ElementLibrary->Items->Strings[ElementLibrary->ItemIndex];

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

        TCircuitElement* newElement = CreateElementByType(elementDesc, x, y);
        if (newElement) {
            FElements.push_back(newElement);
            UpdatePaintBoxSize();
            CircuitImage->Repaint();

            StatusBar->Panels->Items[0]->Text = "Добавлен: " + elementDesc;
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

    for (auto element : FElements) {
        element->Calculate();
    }
}

void __fastcall TMainForm::btnResetSimulationClick(TObject *Sender) {
    ResetSimulation();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "Симуляция сброшена.";
}

void TMainForm::ResetSimulation() {
    for (auto element : FElements) {
        TTernaryTrigger* trigger = dynamic_cast<TTernaryTrigger*>(element);
        if (trigger) {
            trigger->Reset();
        }

        TCounter* counter = dynamic_cast<TCounter*>(element);
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

void __fastcall TMainForm::btnClearWorkspaceClick(TObject *Sender) {
    if (Application->MessageBox(L"Очистить всю рабочую область?", L"Подтверждение",
                               MB_YESNO | MB_ICONQUESTION) == ID_YES) {
        for (auto element : FElements) {
            delete element;
        }
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

void __fastcall TMainForm::miDeleteElementClick(TObject *Sender) {
    if (FSelectedElement) {
        auto it = FConnections.begin();
        while (it != FConnections.end()) {
            if ((it->first->Owner == FSelectedElement) || (it->second->Owner == FSelectedElement)) {
                it = FConnections.erase(it);
            } else {
                ++it;
            }
        }

        auto elemIt = std::find(FElements.begin(), FElements.end(), FSelectedElement);
        if (elemIt != FElements.end()) {
            delete *elemIt;
            FElements.erase(elemIt);
        }

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
    for (auto element : FElements) {
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
            Application->MessageBox(L"Ошибка при загрузке схемя", L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }
}

void TMainForm::SaveSchemeToFile(const String& FileName) {
    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    iniFile->WriteInteger("Scheme", "ElementCount", static_cast<int>(FElements.size()));
    iniFile->WriteInteger("Scheme", "ConnectionCount", static_cast<int>(FConnections.size()));
    iniFile->WriteInteger("Scheme", "NextElementId", FNextElementId);

    for (int i = 0; i < FElements.size(); i++) {
        String section = "Element_" + IntToStr(i);
        TCircuitElement* element = FElements[i];

        iniFile->WriteString(section, "Type", ElementTypeToString(element->ElementType));
        iniFile->WriteInteger(section, "Id", element->Id);
        iniFile->WriteInteger(section, "X", element->Bounds.Left);
        iniFile->WriteInteger(section, "Y", element->Bounds.Top);
        iniFile->WriteInteger(section, "Width", element->Bounds.Width());
        iniFile->WriteInteger(section, "Height", element->Bounds.Height());
        iniFile->WriteString(section, "Name", element->Name);

        iniFile->WriteInteger(section, "InputCount", static_cast<int>(element->Inputs.size()));
        for (int j = 0; j < element->Inputs.size(); j++) {
            String key = "Input_" + IntToStr(j);
            iniFile->WriteInteger(section, key + "_X", element->Inputs[j].X);
            iniFile->WriteInteger(section, key + "_Y", element->Inputs[j].Y);
            iniFile->WriteInteger(section, key + "_Value", static_cast<int>(element->Inputs[j].Value));
            iniFile->WriteBool(section, key + "_IsInput", element->Inputs[j].IsInput);
            iniFile->WriteInteger(section, key + "_LineStyle", static_cast<int>(element->Inputs[j].LineStyle));
        }

        iniFile->WriteInteger(section, "OutputCount", static_cast<int>(element->Outputs.size()));
        for (int j = 0; j < element->Outputs.size(); j++) {
            String key = "Output_" + IntToStr(j);
            iniFile->WriteInteger(section, key + "_X", element->Outputs[j].X);
            iniFile->WriteInteger(section, key + "_Y", element->Outputs[j].Y);
            iniFile->WriteInteger(section, key + "_Value", static_cast<int>(element->Outputs[j].Value));
            iniFile->WriteBool(section, key + "_IsInput", element->Outputs[j].IsInput);
            iniFile->WriteInteger(section, key + "_LineStyle", static_cast<int>(element->Outputs[j].LineStyle));
        }
    }

    for (int i = 0; i < FConnections.size(); i++) {
        String section = "Connection_" + IntToStr(i);
        auto& connection = FConnections[i];

        int fromElementIndex = -1;
        int toElementIndex = -1;

        for (int j = 0; j < FElements.size(); j++) {
            if (connection.first->Owner == FElements[j]) {
                fromElementIndex = j;
            }
            if (connection.second->Owner == FElements[j]) {
                toElementIndex = j;
            }
        }

        if (fromElementIndex != -1 && toElementIndex != -1) {
            iniFile->WriteInteger(section, "FromElement", fromElementIndex);
            iniFile->WriteInteger(section, "ToElement", toElementIndex);

            int fromPointIndex = -1;
            int toPointIndex = -1;

            TCircuitElement* fromElement = FElements[fromElementIndex];
            TCircuitElement* toElement = FElements[toElementIndex];

            for (int j = 0; j < fromElement->Outputs.size(); j++) {
                if (&fromElement->Outputs[j] == connection.first) {
                    fromPointIndex = j;
                    break;
                }
            }

            for (int j = 0; j < toElement->Inputs.size(); j++) {
                if (&toElement->Inputs[j] == connection.second) {
                    toPointIndex = j;
                    break;
                }
            }

            if (fromPointIndex != -1 && toPointIndex != -1) {
                iniFile->WriteInteger(section, "FromPoint", fromPointIndex);
                iniFile->WriteInteger(section, "ToPoint", toPointIndex);
            }
        }
    }

    iniFile->UpdateFile();
}

void TMainForm::LoadSchemeFromFile(const String& FileName) {
    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    btnClearWorkspaceClick(nullptr);

    int elementCount = iniFile->ReadInteger("Scheme", "ElementCount", 0);
    int connectionCount = iniFile->ReadInteger("Scheme", "ConnectionCount", 0);
    FNextElementId = iniFile->ReadInteger("Scheme", "NextElementId", 1);

    std::vector<TCircuitElement*> loadedElements;
    std::vector<std::vector<TConnectionPoint*>> elementInputs;
    std::vector<std::vector<TConnectionPoint*>> elementOutputs;

    for (int i = 0; i < elementCount; i++) {
        String section = "Element_" + IntToStr(i);

        String typeStr = iniFile->ReadString(section, "Type", "");
        int id = iniFile->ReadInteger(section, "Id", 0);
        int x = iniFile->ReadInteger(section, "X", 0);
        int y = iniFile->ReadInteger(section, "Y", 0);
        int width = iniFile->ReadInteger(section, "Width", 80);
        int height = iniFile->ReadInteger(section, "Height", 60);
        String name = iniFile->ReadString(section, "Name", "");

        TCircuitElement* element = CreateElementByTypeName(typeStr, x, y);
        if (element) {
            element->SetId(id);
            element->SetBounds(TRect(x, y, x + width, y + height));
            element->SetName(name);

            int inputCount = iniFile->ReadInteger(section, "InputCount", 0);
            std::vector<TConnectionPoint*> inputs;
            for (int j = 0; j < inputCount; j++) {
                String key = "Input_" + IntToStr(j);
                int inputX = iniFile->ReadInteger(section, key + "_X", 0);
                int inputY = iniFile->ReadInteger(section, key + "_Y", 0);
                TTernary inputValue = static_cast<TTernary>(iniFile->ReadInteger(section, key + "_Value", 0));
                bool inputIsInput = iniFile->ReadBool(section, key + "_IsInput", true);
                TLineStyle inputLineStyle = static_cast<TLineStyle>(iniFile->ReadInteger(section, key + "_LineStyle", 0));

                TConnectionPoint point(element, inputX, inputY, inputValue, inputIsInput, inputLineStyle);
                if (j < element->Inputs.size()) {
                    element->Inputs[j] = point;
                    inputs.push_back(&element->Inputs[j]);
                }
            }

            int outputCount = iniFile->ReadInteger(section, "OutputCount", 0);
            std::vector<TConnectionPoint*> outputs;
            for (int j = 0; j < outputCount; j++) {
                String key = "Output_" + IntToStr(j);
                int outputX = iniFile->ReadInteger(section, key + "_X", 0);
                int outputY = iniFile->ReadInteger(section, key + "_Y", 0);
                TTernary outputValue = static_cast<TTernary>(iniFile->ReadInteger(section, key + "_Value", 0));
                bool outputIsInput = iniFile->ReadBool(section, key + "_IsInput", false);
                TLineStyle outputLineStyle = static_cast<TLineStyle>(iniFile->ReadInteger(section, key + "_LineStyle", 0));

                TConnectionPoint point(element, outputX, outputY, outputValue, outputIsInput, outputLineStyle);
                if (j < element->Outputs.size()) {
                    element->Outputs[j] = point;
                    outputs.push_back(&element->Outputs[j]);
                }
            }

            loadedElements.push_back(element);
            elementInputs.push_back(inputs);
            elementOutputs.push_back(outputs);
            FElements.push_back(element);

            element->CalculateRelativePositions();
        }
    }

    for (int i = 0; i < connectionCount; i++) {
        String section = "Connection_" + IntToStr(i);

        int fromElementIndex = iniFile->ReadInteger(section, "FromElement", -1);
        int toElementIndex = iniFile->ReadInteger(section, "ToElement", -1);
        int fromPointIndex = iniFile->ReadInteger(section, "FromPoint", -1);
        int toPointIndex = iniFile->ReadInteger(section, "ToPoint", -1);

        if (fromElementIndex >= 0 && fromElementIndex < loadedElements.size() &&
            toElementIndex >= 0 && toElementIndex < loadedElements.size() &&
            fromPointIndex >= 0 && fromPointIndex < elementOutputs[fromElementIndex].size() &&
            toPointIndex >= 0 && toPointIndex < elementInputs[toElementIndex].size()) {

            TConnectionPoint* fromPoint = elementOutputs[fromElementIndex][fromPointIndex];
            TConnectionPoint* toPoint = elementInputs[toElementIndex][toPointIndex];

            FConnections.push_back(std::make_pair(fromPoint, toPoint));
        }
    }

    UpdatePaintBoxSize();
    CircuitImage->Repaint();
}

String TMainForm::ElementTypeToString(TElementType Type) {
    switch (Type) {
        case TElementType::MAGNETIC_AMPLIFIER: return "MAGNETIC_AMPLIFIER";
        case TElementType::MAGNETIC_AMPLIFIER_POWER: return "MAGNETIC_AMPLIFIER_POWER";
        case TElementType::TERNARY_ELEMENT: return "TERNARY_ELEMENT";
        case TElementType::TERNARY_TRIGGER: return "TERNARY_TRIGGER";
        case TElementType::TERNARY_ADDER: return "TERNARY_ADDER";
        case TElementType::HALF_ADDER: return "HALF_ADDER";
        case TElementType::SHIFT_REGISTER: return "SHIFT_REGISTER";
        case TElementType::DECODER: return "DECODER";
        case TElementType::COUNTER: return "COUNTER";
        case TElementType::LOGIC_AND: return "LOGIC_AND";
        case TElementType::LOGIC_OR: return "LOGIC_OR";
        case TElementType::LOGIC_INHIBIT: return "LOGIC_INHIBIT";
        case TElementType::GENERATOR: return "GENERATOR";
        case TElementType::SUBCIRCUIT: return "SUBCIRCUIT";
        default: return "UNKNOWN";
    }
}

TElementType TMainForm::StringToElementType(const String& TypeStr) {
    if (TypeStr == "MAGNETIC_AMPLIFIER") return TElementType::MAGNETIC_AMPLIFIER;
    if (TypeStr == "MAGNETIC_AMPLIFIER_POWER") return TElementType::MAGNETIC_AMPLIFIER_POWER;
    if (TypeStr == "TERNARY_ELEMENT") return TElementType::TERNARY_ELEMENT;
    if (TypeStr == "TERNARY_TRIGGER") return TElementType::TERNARY_TRIGGER;
    if (TypeStr == "TERNARY_ADDER") return TElementType::TERNARY_ADDER;
    if (TypeStr == "HALF_ADDER") return TElementType::HALF_ADDER;
    if (TypeStr == "SHIFT_REGISTER") return TElementType::SHIFT_REGISTER;
    if (TypeStr == "DECODER") return TElementType::DECODER;
    if (TypeStr == "COUNTER") return TElementType::COUNTER;
    if (TypeStr == "LOGIC_AND") return TElementType::LOGIC_AND;
    if (TypeStr == "LOGIC_OR") return TElementType::LOGIC_OR;
    if (TypeStr == "LOGIC_INHIBIT") return TElementType::LOGIC_INHIBIT;
    if (TypeStr == "GENERATOR") return TElementType::GENERATOR;
    if (TypeStr == "SUBCIRCUIT") return TElementType::SUBCIRCUIT;
    return TElementType::TERNARY_ELEMENT;
}

TCircuitElement* TMainForm::CreateElementByTypeName(const String& TypeName, int X, int Y) {
    TElementType type = StringToElementType(TypeName);

    TCircuitElement* element = nullptr;

    switch (type) {
        case TElementType::MAGNETIC_AMPLIFIER:
            element = new TMagneticAmplifier(FNextElementId++, X, Y, false);
            break;
        case TElementType::MAGNETIC_AMPLIFIER_POWER:
            element = new TMagneticAmplifier(FNextElementId++, X, Y, true);
            break;
        case TElementType::TERNARY_ELEMENT:
            element = new TTernaryElement(FNextElementId++, X, Y);
            break;
        case TElementType::TERNARY_TRIGGER:
            element = new TTernaryTrigger(FNextElementId++, X, Y);
            break;
        case TElementType::HALF_ADDER:
            element = new THalfAdder(FNextElementId++, X, Y);
            break;
        case TElementType::TERNARY_ADDER:
            element = new TTernaryAdder(FNextElementId++, X, Y);
            break;
        case TElementType::DECODER:
            element = new TDecoder(FNextElementId++, X, Y, 2);
            break;
        case TElementType::COUNTER:
            element = new TCounter(FNextElementId++, X, Y, 3);
            break;
        case TElementType::SHIFT_REGISTER:
            element = new TShiftRegister(FNextElementId++, X, Y, 4);
            break;
        case TElementType::LOGIC_AND:
            element = new TLogicAnd(FNextElementId++, X, Y);
            break;
        case TElementType::LOGIC_OR:
            element = new TLogicOr(FNextElementId++, X, Y);
            break;
        case TElementType::LOGIC_INHIBIT:
            element = new TLogicInhibit(FNextElementId++, X, Y);
            break;
        case TElementType::GENERATOR:
            element = new TGenerator(FNextElementId++, X, Y);
            break;
        default:
            element = new TTernaryElement(FNextElementId++, X, Y);
    }

    if (element) {
        element->CalculateRelativePositions();
    }
    return element;
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

    TSubCircuit* subCircuit = new TSubCircuit(FNextElementId++, centerX, centerY,
                                             selectedElements, internalConnections);

    for (auto element : selectedElements) {
        auto it = std::find(FElements.begin(), FElements.end(), element);
        if (it != FElements.end()) {
            FElements.erase(it);
        }
    }

    for (auto& internalConn : internalConnections) {
        auto it = std::find(FConnections.begin(), FConnections.end(), internalConn);
        if (it != FConnections.end()) {
            FConnections.erase(it);
        }
    }

    FElements.push_back(subCircuit);
    FSelectedElement = subCircuit;
    FSelectedElements.clear();
    FSelectedElements.push_back(subCircuit);

    UpdatePaintBoxSize();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "Создана подсхема из " + IntToStr(static_cast<int>(selectedElements.size())) + " элементов";
}

void TMainForm::UngroupSubCircuit(TCircuitElement* SubCircuit) {
    TSubCircuit* subCircuit = dynamic_cast<TSubCircuit*>(SubCircuit);
    if (!subCircuit) return;

    const auto& internalElements = subCircuit->GetInternalElements();
    const auto& internalConnections = subCircuit->GetInternalConnections();

    for (auto element : internalElements) {
        FElements.push_back(element);
    }

    for (auto& connection : internalConnections) {
        FConnections.push_back(connection);
    }

    auto it = std::find(FElements.begin(), FElements.end(), SubCircuit);
    if (it != FElements.end()) {
        FElements.erase(it);
    }

    FSelectedElement = nullptr;
    FSelectedElements.clear();

    delete SubCircuit;

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