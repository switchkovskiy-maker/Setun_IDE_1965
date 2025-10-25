#include "MainForm.h"
#include "CircuitElement.h"
#include "CircuitElements.h"
#include <Vcl.Dialogs.hpp>
#include <Vcl.Graphics.hpp>
#include <math.h>
#include <algorithm>
#include <memory>

#pragma package(smart_init)
#pragma resource "*.dfm"

TMainForm *MainForm;

__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner),
    FSelectedElement(nullptr), FDraggedElement(nullptr), FConnectionStart(nullptr),
    FIsConnecting(false), FIsDragging(false), FNextElementId(1),
    FDragOffsetX(0), FDragOffsetY(0), FZoomFactor(1.0) {
}

void __fastcall TMainForm::FormCreate(TObject *Sender) {
    CreateCompleteLibrary();

    // Настройка workspace
    CircuitImage->Align = alNone;
    CircuitImage->Cursor = crCross;
    UpdatePaintBoxSize();

    // Настройка панели инструментов
    btnConnectionMode->AllowAllUp = true;
    btnConnectionMode->GroupIndex = 1;

    // Настройка статусной строки
    StatusBar->Panels->Items[0]->Text = "Готов к работе. Выберите элемент из библиотеки или режим соединения.";

    // Установка заголовка
    Caption = "Setun IDE - Троичная логика по книге 1965 года";
}

void __fastcall TMainForm::FormDestroy(TObject *Sender) {
    for (auto element : FElements) {
        delete element;
    }
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
    if (FElements.empty()) {
        // Минимальный размер, если нет элементов
        CircuitImage->Width = Workspace->ClientWidth;
        CircuitImage->Height = Workspace->ClientHeight;
    } else {
        // Вычисляем границы всех элементов
        TRect bounds = GetCircuitBounds();

        // Добавляем отступы
        int padding = 100;
        int newWidth = bounds.Width() + padding * 2;
        int newHeight = bounds.Height() + padding * 2;

        // Обеспечиваем минимальный размер
        if (newWidth < Workspace->ClientWidth) newWidth = Workspace->ClientWidth;
        if (newHeight < Workspace->ClientHeight) newHeight = Workspace->ClientHeight;

        CircuitImage->Width = newWidth;
        CircuitImage->Height = newHeight;

        // Сдвигаем все элементы, если они выходят за левую/верхнюю границу
        if (bounds.Left < padding || bounds.Top < padding) {
            int deltaX = padding - bounds.Left;
            int deltaY = padding - bounds.Top;

            for (auto element : FElements) {
                TRect newBounds = element->Bounds;
                newBounds.Left += deltaX;
                newBounds.Top += deltaY;
                newBounds.Right += deltaX;
                newBounds.Bottom += deltaY;
                element->SetBounds(newBounds);
            }
        }
    }
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

void TMainForm::CreateCompleteLibrary() {
    ElementLibrary->Items->Clear();

    // Полная библиотека элементов по книге 1965 года
    ElementLibrary->Items->Add("Магнитный усилитель (простой) - стр. 43");
    ElementLibrary->Items->Add("Магнитный усилитель (мощный) - стр. 49");
    ElementLibrary->Items->Add("Троичный элемент - стр. 50");
    ElementLibrary->Items->Add("Троичный триггер - стр. 55");
    ElementLibrary->Items->Add("Полусумматор - стр. 58");
    ElementLibrary->Items->Add("Троичный сумматор - стр. 58");
    ElementLibrary->Items->Add("Дешифратор кода - стр. 57");
    ElementLibrary->Items->Add("Троичный счетчик - стр. 59");
    ElementLibrary->Items->Add("Сдвигающий регистр - стр. 45");
    ElementLibrary->Items->Add("Логический элемент И - стр. 49");
    ElementLibrary->Items->Add("Логический элемент ИЛИ - стр. 49");
    ElementLibrary->Items->Add("Схема запрета - стр. 47");
    ElementLibrary->Items->Add("Генератор единиц - стр. 50");

    LibraryLabel->Caption = "Библиотека элементов (Сетунь, 1965)";
}

TCircuitElement* TMainForm::CreateElementByType(const String& ElementDesc, int X, int Y) {
    if (ElementDesc.Pos("Магнитный усилитель (простой)") > 0) {
        return new TMagneticAmplifier(FNextElementId++, X, Y, false);
    } else if (ElementDesc.Pos("Магнитный усилитель (мощный)") > 0) {
        return new TMagneticAmplifier(FNextElementId++, X, Y, true);
    } else if (ElementDesc.Pos("Троичный элемент") > 0) {
        return new TTernaryElement(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("Троичный триггер") > 0) {
        return new TTernaryTrigger(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("Полусумматор") > 0) {
        return new THalfAdder(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("Троичный сумматор") > 0) {
        return new TTernaryAdder(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("Дешифратор кода") > 0) {
        return new TDecoder(FNextElementId++, X, Y, 2);
    } else if (ElementDesc.Pos("Троичный счетчик") > 0) {
        return new TCounter(FNextElementId++, X, Y, 3);
    } else if (ElementDesc.Pos("Сдвигающий регистр") > 0) {
        return new TShiftRegister(FNextElementId++, X, Y, 4);
    } else if (ElementDesc.Pos("Логический элемент И") > 0) {
        return new TLogicAnd(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("Логический элемент ИЛИ") > 0) {
        return new TLogicOr(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("Схема запрета") > 0) {
        return new TLogicInhibit(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("Генератор единиц") > 0) {
        return new TGenerator(FNextElementId++, X, Y);
    }

    return nullptr;
}

void __fastcall TMainForm::CircuitImagePaint(TObject *Sender) {
    DrawCircuit();
}

void TMainForm::DrawCircuit() {
    // Создаем буферизованный bitmap для избежания мерцания
    std::unique_ptr<TBitmap> buffer(new TBitmap);
    buffer->Width = CircuitImage->Width;
    buffer->Height = CircuitImage->Height;

    TCanvas* canvas = buffer->Canvas;

    // Очистка фона
    canvas->Brush->Color = clWhite;
    canvas->FillRect(CircuitImage->ClientRect);

    // Рисуем сетку для удобства позиционирования
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

    // Рисуем соединения
    canvas->Pen->Width = 2;
    for (auto& connection : FConnections) {
        TConnectionPoint* start = connection.first;
        TConnectionPoint* end = connection.second;

        // Устанавливаем цвет по значению
        canvas->Pen->Color = TernaryToColor(start->Value);

        canvas->MoveTo(start->X, start->Y);
        canvas->LineTo(end->X, end->Y);

        // Рисуем стрелку на конце соединения
        int dx = end->X - start->X;
        int dy = end->Y - start->Y;
        double length = sqrt(dx*dx + dy*dy);
        if (length > 0) {
            double unitX = dx / length;
            double unitY = dy / length;

            // Стрелка
            int arrowSize = 6;
            int arrowX = end->X - (int)(unitX * arrowSize);
            int arrowY = end->Y - (int)(unitY * arrowSize);

            canvas->MoveTo(arrowX - (int)(unitY * arrowSize/2), arrowY + (int)(unitX * arrowSize/2));
            canvas->LineTo(end->X, end->Y);
            canvas->LineTo(arrowX + (int)(unitY * arrowSize/2), arrowY - (int)(unitX * arrowSize/2));
        }
    }
    canvas->Pen->Width = 1;

    // Рисуем элементы
    for (auto element : FElements) {
        element->Draw(canvas);
    }

    // Подсвечиваем выбранный элемент
    if (FSelectedElement) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Width = 2;
        canvas->Pen->Style = psDash;
        canvas->Brush->Style = bsClear;
        canvas->Rectangle(FSelectedElement->Bounds);
        canvas->Pen->Style = psSolid;
        canvas->Pen->Width = 1;
        canvas->Brush->Style = bsSolid;
    }

    // Показываем временное соединение в режиме подключения
    if (FIsConnecting && FConnectionStart) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Style = psDash;
        canvas->MoveTo(FConnectionStart->X, FConnectionStart->Y);
        TPoint mousePos = CircuitImage->ScreenToClient(Mouse->CursorPos);
        canvas->LineTo(mousePos.X, mousePos.Y);
        canvas->Pen->Style = psSolid;
    }

    // Копируем буфер на экран
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

void __fastcall TMainForm::CircuitImageMouseDown(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y) {

    if (Button == mbLeft) {
        // Проверяем режим соединения
        if (btnConnectionMode->Down) {
            // Режим создания соединений
            for (auto element : FElements) {
                TConnectionPoint* conn = element->GetConnectionAt(X, Y);
                if (conn) {
                    if (!FIsConnecting) {
                        // Первая точка соединения - должна быть выходом
                        if (!conn->IsInput) {
                            FConnectionStart = conn;
                            FIsConnecting = true;
                            StatusBar->Panels->Items[0]->Text = "Выбрана выходная точка. Выберите входную точку.";
                        } else {
                            StatusBar->Panels->Items[0]->Text = "Ошибка: первая точка должна быть выходом (синяя)";
                        }
                        CircuitImage->Repaint();
                    } else {
                        // Вторая точка соединения - должна быть входом
                        if (conn->IsInput) {
                            // Проверяем, что соединение не дублируется
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

            // Если клик не на точке соединения, сбрасываем соединение
            if (FIsConnecting) {
                FIsConnecting = false;
                FConnectionStart = nullptr;
                StatusBar->Panels->Items[0]->Text = "Соединение отменено.";
                CircuitImage->Repaint();
            }
        } else {
            // Режим выбора и перемещения элементов
            FSelectedElement = nullptr;
            FDraggedElement = nullptr;

            // Ищем элемент под курсором (включая проверку границ)
            for (auto element : FElements) {
                TRect bounds = element->Bounds;
                // Небольшое расширение области для удобства захвата
                TRect expandedBounds = TRect(
                    bounds.Left - 5, bounds.Top - 5,
                    bounds.Right + 5, bounds.Bottom + 5
                );

                if (X >= expandedBounds.Left && X <= expandedBounds.Right &&
                    Y >= expandedBounds.Top && Y <= expandedBounds.Bottom) {

                    FSelectedElement = element;
                    FDraggedElement = element;
                    FIsDragging = true;

                    // Смещение относительно левого верхнего угла элемента
                    FDragOffsetX = X - bounds.Left;
                    FDragOffsetY = Y - bounds.Top;

                    StatusBar->Panels->Items[0]->Text = "Выбран элемент: " + element->Name;
                    break;
                }
            }

            if (!FSelectedElement) {
                StatusBar->Panels->Items[0]->Text = "Ничего не выбрано.";
            }

            CircuitImage->Repaint();
        }
    } else if (Button == mbRight) {
        // Правый клик для контекстного меню
        FSelectedElement = nullptr;
        for (auto element : FElements) {
            if (X >= element->Bounds.Left && X <= element->Bounds.Right &&
                Y >= element->Bounds.Top && Y <= element->Bounds.Bottom) {
                FSelectedElement = element;

                // Показываем контекстное меню
                TPoint popupPos = CircuitImage->ClientToScreen(TPoint(X, Y));
                ElementPopupMenu->Popup(popupPos.X, popupPos.Y);
                break;
            }
        }
    }
}

void __fastcall TMainForm::CircuitImageMouseMove(TObject *Sender, TShiftState Shift, int X, int Y) {
    if (FIsDragging && FDraggedElement) {
        // Новые координаты элемента
        int newLeft = X - FDragOffsetX;
        int newTop = Y - FDragOffsetY;

        // Создаем новые границы с сохранением размера
        TRect newBounds = TRect(
            newLeft,
            newTop,
            newLeft + FDraggedElement->Bounds.Width(),
            newTop + FDraggedElement->Bounds.Height()
        );

        // Обновляем позицию элемента (SetBounds сам обновит соединения)
        FDraggedElement->SetBounds(newBounds);

        // Обновляем размер области рисования при необходимости
        UpdatePaintBoxSize();

        // Немедленная перерисовка
        CircuitImage->Repaint();
    }

    // Обновляем курсор в режиме соединения
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
        FIsDragging = false;
        FDraggedElement = nullptr;
    }
}

void __fastcall TMainForm::ElementLibraryDblClick(TObject *Sender) {
    if (ElementLibrary->ItemIndex >= 0) {
        String elementDesc = ElementLibrary->Items->Strings[ElementLibrary->ItemIndex];

        // Позиция для нового элемента (центр workspace или рядом с курсором)
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
    // Сначала обновляем входы элементов из соединений
    for (auto& connection : FConnections) {
        if (connection.first && connection.second) {
            // Передаем значение от выхода ко входу
            connection.second->Value = connection.first->Value;
        }
    }

    // Затем вычисляем все элементы
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
    // Сбрасываем все элементы в нулевое состояние
    for (auto element : FElements) {
        // Для элементов, которые имеют метод сброса (например, триггеры, счетчики)
        TTernaryTrigger* trigger = dynamic_cast<TTernaryTrigger*>(element);
        if (trigger) {
            trigger->Reset();
        }

        TCounter* counter = dynamic_cast<TCounter*>(element);
        if (counter) {
            counter->Reset();
        }

        // Сбрасываем входы и выходы
        for (auto& input : element->Inputs) {
            input.Value = TTernary::ZERO;
        }
        for (auto& output : element->Outputs) {
            output.Value = TTernary::ZERO;
        }
    }

    // Сбрасываем соединения
    for (auto& connection : FConnections) {
        if (connection.first) connection.first->Value = TTernary::ZERO;
        if (connection.second) connection.second->Value = TTernary::ZERO;
    }
}

void __fastcall TMainForm::btnClearWorkspaceClick(TObject *Sender) {
    if (Application->MessageBox(L"Очистить всю рабочую область?", L"Подтверждение",
                               MB_YESNO | MB_ICONQUESTION) == ID_YES) {
        // Удаляем все элементы
        for (auto element : FElements) {
            delete element;
        }
        FElements.clear();

        // Очищаем соединения
        FConnections.clear();

        // Сбрасываем счетчик ID
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
        // Удаляем соединения, связанные с этим элементом
        auto it = FConnections.begin();
        while (it != FConnections.end()) {
            if ((it->first->X >= FSelectedElement->Bounds.Left - 20 &&
                it->first->X <= FSelectedElement->Bounds.Right + 20 &&
                it->first->Y >= FSelectedElement->Bounds.Top - 20 &&
                it->first->Y <= FSelectedElement->Bounds.Bottom + 20) ||
                (it->second->X >= FSelectedElement->Bounds.Left - 20 &&
                it->second->X <= FSelectedElement->Bounds.Right + 20 &&
                it->second->Y >= FSelectedElement->Bounds.Top - 20 &&
                it->second->Y <= FSelectedElement->Bounds.Bottom + 20)) {
                it = FConnections.erase(it);
            } else {
                ++it;
            }
        }

        // Удаляем элемент
        auto elemIt = std::find(FElements.begin(), FElements.end(), FSelectedElement);
        if (elemIt != FElements.end()) {
            delete *elemIt;
            FElements.erase(elemIt);
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

    // Информация о состоянии
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
        // Простая реализация поворота - меняем местами ширину и высоту
        TRect newBounds = FSelectedElement->Bounds;
        int width = newBounds.Width();
        int height = newBounds.Height();

        // Меняем размеры (это упрощенная реализация)
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
    StatusBar->Panels->Items[0]->Text = "Увеличение сброшено до 100%";
}

void TMainForm::ApplyZoom() {
    // Масштабируем все элементы
    for (auto element : FElements) {
        TRect bounds = element->Bounds;
        bounds.Left = (int)(bounds.Left * FZoomFactor);
        bounds.Top = (int)(bounds.Top * FZoomFactor);
        bounds.Right = (int)(bounds.Right * FZoomFactor);
        bounds.Bottom = (int)(bounds.Bottom * FZoomFactor);
        element->SetBounds(bounds);
    }

    // Обновляем размер области рисования
    UpdatePaintBoxSize();
    CircuitImage->Repaint();
}
