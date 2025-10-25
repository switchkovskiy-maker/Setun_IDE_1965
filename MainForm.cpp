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
    FDragOffsetX(0), FDragOffsetY(0) {
}

void __fastcall TMainForm::FormCreate(TObject *Sender) {
    CreateCompleteLibrary();

    // ��������� workspace
    CircuitImage->Align = alNone;
    CircuitImage->Cursor = crCross;
    UpdatePaintBoxSize();

    // ��������� ������ ������������
    btnConnectionMode->AllowAllUp = true;
    btnConnectionMode->GroupIndex = 1;

    // ��������� ��������� ������
    StatusBar->Panels->Items[0]->Text = "����� � ������. �������� ������� �� ���������� ��� ����� ����������.";

    // ��������� ���������
    Caption = "Setun IDE - �������� ������ �� ����� 1965 ����";
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
        // ����������� ������
        CircuitImage->Width = std::max(Workspace->ClientWidth, 800);
        CircuitImage->Height = std::max(Workspace->ClientHeight, 600);
    } else {
        // ��������� ������� ���� ���������
        TRect bounds = GetCircuitBounds();

        // ��������� �������
        int padding = 200;
        int newWidth = bounds.Width() + padding;
        int newHeight = bounds.Height() + padding;

        // ������������ ����������� ������
        newWidth = std::max(newWidth, Workspace->ClientWidth);
        newHeight = std::max(newHeight, Workspace->ClientHeight);

        CircuitImage->Width = newWidth;
        CircuitImage->Height = newHeight;
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

    // ������ ���������� ��������� �� ����� 1965 ����
    ElementLibrary->Items->Add("��������� ��������� (�������) - ���. 43");
    ElementLibrary->Items->Add("��������� ��������� (������) - ���. 49");
    ElementLibrary->Items->Add("�������� ������� - ���. 50");
    ElementLibrary->Items->Add("�������� ������� - ���. 55");
    ElementLibrary->Items->Add("������������ - ���. 58");
    ElementLibrary->Items->Add("�������� �������� - ���. 58");
    ElementLibrary->Items->Add("���������� ���� - ���. 57");
    ElementLibrary->Items->Add("�������� ������� - ���. 59");
    ElementLibrary->Items->Add("���������� ������� - ���. 45");
    ElementLibrary->Items->Add("���������� ������� � - ���. 49");
    ElementLibrary->Items->Add("���������� ������� ��� - ���. 49");
    ElementLibrary->Items->Add("����� ������� - ���. 47");
    ElementLibrary->Items->Add("��������� ������ - ���. 50");

    LibraryLabel->Caption = "���������� ��������� (������, 1965)";
}

TCircuitElement* TMainForm::CreateElementByType(const String& ElementDesc, int X, int Y) {
    if (ElementDesc.Pos("��������� ��������� (�������)") > 0) {
        return new TMagneticAmplifier(FNextElementId++, X, Y, false);
    } else if (ElementDesc.Pos("��������� ��������� (������)") > 0) {
        return new TMagneticAmplifier(FNextElementId++, X, Y, true);
    } else if (ElementDesc.Pos("�������� �������") > 0) {
        return new TTernaryElement(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("�������� �������") > 0) {
        return new TTernaryTrigger(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("������������") > 0) {
        return new THalfAdder(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("�������� ��������") > 0) {
        return new TTernaryAdder(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("���������� ����") > 0) {
        return new TDecoder(FNextElementId++, X, Y, 2);
    } else if (ElementDesc.Pos("�������� �������") > 0) {
        return new TCounter(FNextElementId++, X, Y, 3);
    } else if (ElementDesc.Pos("���������� �������") > 0) {
        return new TShiftRegister(FNextElementId++, X, Y, 4);
    } else if (ElementDesc.Pos("���������� ������� �") > 0) {
        return new TLogicAnd(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("���������� ������� ���") > 0) {
        return new TLogicOr(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("����� �������") > 0) {
        return new TLogicInhibit(FNextElementId++, X, Y);
    } else if (ElementDesc.Pos("��������� ������") > 0) {
        return new TGenerator(FNextElementId++, X, Y);
    }

    return nullptr;
}

void __fastcall TMainForm::CircuitImagePaint(TObject *Sender) {
    DrawCircuit();
}

void TMainForm::DrawCircuit() {
    // ������� �������������� bitmap ��� ��������� ��������
    std::unique_ptr<TBitmap> buffer(new TBitmap);
    buffer->Width = CircuitImage->Width;
    buffer->Height = CircuitImage->Height;

    TCanvas* canvas = buffer->Canvas;

    // ������� ����
    canvas->Brush->Color = clWhite;
    canvas->FillRect(CircuitImage->ClientRect);

    // ������ ����� ��� �������� ����������������
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

    // ������ ����������
    canvas->Pen->Width = 2;
    for (auto& connection : FConnections) {
        TConnectionPoint* start = connection.first;
        TConnectionPoint* end = connection.second;

        // ������������� ���� �� ��������
        canvas->Pen->Color = TernaryToColor(start->Value);

        canvas->MoveTo(start->X, start->Y);
        canvas->LineTo(end->X, end->Y);

        // ������ ������� �� ����� ����������
        int dx = end->X - start->X;
        int dy = end->Y - start->Y;
        double length = sqrt(dx*dx + dy*dy);
        if (length > 0) {
            double unitX = dx / length;
            double unitY = dy / length;

            // �������
            int arrowSize = 6;
            int arrowX = end->X - (int)(unitX * arrowSize);
            int arrowY = end->Y - (int)(unitY * arrowSize);

            canvas->MoveTo(arrowX - (int)(unitY * arrowSize/2), arrowY + (int)(unitX * arrowSize/2));
            canvas->LineTo(end->X, end->Y);
            canvas->LineTo(arrowX + (int)(unitY * arrowSize/2), arrowY - (int)(unitX * arrowSize/2));
        }
    }
    canvas->Pen->Width = 1;

    // ������ ��������
    for (auto element : FElements) {
        element->Draw(canvas);
    }

    // ������������ ��������� �������
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

    // ���������� ��������� ���������� � ������ �����������
    if (FIsConnecting && FConnectionStart) {
        canvas->Pen->Color = clBlue;
        canvas->Pen->Style = psDash;
        canvas->MoveTo(FConnectionStart->X, FConnectionStart->Y);
        TPoint mousePos = CircuitImage->ScreenToClient(Mouse->CursorPos);
        canvas->LineTo(mousePos.X, mousePos.Y);
        canvas->Pen->Style = psSolid;
    }

    // �������� ����� �� �����
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
        // ��������� ����� ����������
        if (btnConnectionMode->Down) {
            // ����� �������� ����������
            for (auto element : FElements) {
                TConnectionPoint* conn = element->GetConnectionAt(X, Y);
                if (conn) {
                    if (!FIsConnecting) {
                        // ������ ����� ���������� - ������ ���� �������
                        if (!conn->IsInput) {
                            FConnectionStart = conn;
                            FIsConnecting = true;
                            StatusBar->Panels->Items[0]->Text = "������� �������� �����. �������� ������� �����.";
                        } else {
                            StatusBar->Panels->Items[0]->Text = "������: ������ ����� ������ ���� ������� (�����)";
                        }
                        CircuitImage->Repaint();
                    } else {
                        // ������ ����� ���������� - ������ ���� ������
                        if (conn->IsInput) {
                            // ���������, ��� ���������� �� �����������
                            bool connectionExists = false;
                            for (auto& existingConn : FConnections) {
                                if (existingConn.first == FConnectionStart && existingConn.second == conn) {
                                    connectionExists = true;
                                    break;
                                }
                            }

                            if (!connectionExists) {
                                FConnections.push_back(std::make_pair(FConnectionStart, conn));
                                StatusBar->Panels->Items[0]->Text = "���������� �������.";
                            } else {
                                StatusBar->Panels->Items[0]->Text = "���������� ��� ����������.";
                            }
                        } else {
                            StatusBar->Panels->Items[0]->Text = "������: ������ ����� ������ ���� ������ (�������)";
                        }
                        FIsConnecting = false;
                        FConnectionStart = nullptr;
                        CircuitImage->Repaint();
                    }
                    return;
                }
            }

            // ���� ���� �� �� ����� ����������, ���������� ����������
            if (FIsConnecting) {
                FIsConnecting = false;
                FConnectionStart = nullptr;
                StatusBar->Panels->Items[0]->Text = "���������� ��������.";
                CircuitImage->Repaint();
            }
        } else {
            // ����� ������ � ����������� ���������
            FSelectedElement = nullptr;
            FDraggedElement = nullptr;

            // ���� ������� ��� �������� (������� �������� ������)
            for (auto element : FElements) {
                TRect bounds = element->Bounds;
                // ��������� ���������� ������� ��� �������� �������
                TRect expandedBounds = TRect(
                    bounds.Left - 5, bounds.Top - 5,
                    bounds.Right + 5, bounds.Bottom + 5
                );

                if (X >= expandedBounds.Left && X <= expandedBounds.Right &&
                    Y >= expandedBounds.Top && Y <= expandedBounds.Bottom) {

                    FSelectedElement = element;
                    FDraggedElement = element;
                    FIsDragging = true;

                    // �������� ������������ ������ �������� ���� ��������
                    FDragOffsetX = X - bounds.Left;
                    FDragOffsetY = Y - bounds.Top;

                    StatusBar->Panels->Items[0]->Text = "������ �������: " + element->Name;
                    break;
                }
            }

            if (!FSelectedElement) {
                StatusBar->Panels->Items[0]->Text = "������ �� �������.";
            }

            CircuitImage->Repaint();
        }
    } else if (Button == mbRight) {
        // ������ ���� ��� ������������ ����
        FSelectedElement = nullptr;
        for (auto element : FElements) {
            if (X >= element->Bounds.Left && X <= element->Bounds.Right &&
                Y >= element->Bounds.Top && Y <= element->Bounds.Bottom) {
                FSelectedElement = element;

                // ���������� ����������� ����
                TPoint popupPos = CircuitImage->ClientToScreen(TPoint(X, Y));
                ElementPopupMenu->Popup(popupPos.X, popupPos.Y);
                break;
            }
        }
    }
}

void __fastcall TMainForm::CircuitImageMouseMove(TObject *Sender, TShiftState Shift, int X, int Y) {
    if (FIsDragging && FDraggedElement) {
        // ����� ���������� ��������
        int newLeft = X - FDragOffsetX;
        int newTop = Y - FDragOffsetY;

        // ������� ����� ������� � ����������� �������
        TRect newBounds = TRect(
            newLeft,
            newTop,
            newLeft + FDraggedElement->Bounds.Width(),
            newTop + FDraggedElement->Bounds.Height()
        );

        // ��������� ������� �������� (SetBounds ��� ������� ����������)
        FDraggedElement->SetBounds(newBounds);

        // ��������� ������ ������� ��������� ��� �������������
        UpdatePaintBoxSize();

        // ����������� �����������
        CircuitImage->Repaint();
    }

    // ��������� ������ � ������ ����������
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

        // ������� ��� ������ �������� (����� workspace ��� ����� � ��������)
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

            StatusBar->Panels->Items[0]->Text = "��������: " + elementDesc;
        }
    }
}

void __fastcall TMainForm::btnRunSimulationClick(TObject *Sender) {
    RunSimulationStep();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "��������� ���������.";
}

void TMainForm::RunSimulationStep() {
    // ������� ��������� ����� ��������� �� ����������
    for (auto& connection : FConnections) {
        if (connection.first && connection.second) {
            // �������� �������� �� ������ �� �����
            connection.second->Value = connection.first->Value;
        }
    }

    // ����� ��������� ��� ��������
    for (auto element : FElements) {
        element->Calculate();
    }
}

void __fastcall TMainForm::btnResetSimulationClick(TObject *Sender) {
    ResetSimulation();
    CircuitImage->Repaint();
    StatusBar->Panels->Items[0]->Text = "��������� ��������.";
}

void TMainForm::ResetSimulation() {
    // ���������� ��� �������� � ������� ���������
    for (auto element : FElements) {
        // ��� ���������, ������� ����� ����� ������ (��������, ��������, ��������)
        TTernaryTrigger* trigger = dynamic_cast<TTernaryTrigger*>(element);
        if (trigger) {
            trigger->Reset();
        }

        TCounter* counter = dynamic_cast<TCounter*>(element);
        if (counter) {
            counter->Reset();
        }

        // ���������� ����� � ������
        for (auto& input : element->Inputs) {
            input.Value = TTernary::ZERO;
        }
        for (auto& output : element->Outputs) {
            output.Value = TTernary::ZERO;
        }
    }

    // ���������� ����������
    for (auto& connection : FConnections) {
        if (connection.first) connection.first->Value = TTernary::ZERO;
        if (connection.second) connection.second->Value = TTernary::ZERO;
    }
}

void __fastcall TMainForm::btnClearWorkspaceClick(TObject *Sender) {
    if (Application->MessageBox(L"�������� ��� ������� �������?", L"�������������",
                               MB_YESNO | MB_ICONQUESTION) == ID_YES) {
        // ������� ��� ��������
        for (auto element : FElements) {
            delete element;
        }
        FElements.clear();

        // ������� ����������
        FConnections.clear();

        // ���������� ������� ID
        FNextElementId = 1;

        UpdatePaintBoxSize();
        CircuitImage->Repaint();
        StatusBar->Panels->Items[0]->Text = "������� ������� �������.";
    }
}

void __fastcall TMainForm::btnConnectionModeClick(TObject *Sender) {
    if (btnConnectionMode->Down) {
        StatusBar->Panels->Items[0]->Text = "����� ����������: ��������� ����� ��� ���������� ���������.";
        CircuitImage->Cursor = crCross;
    } else {
        StatusBar->Panels->Items[0]->Text = "����� ������: ��������� � ����������� ��������.";
        CircuitImage->Cursor = crDefault;
        FIsConnecting = false;
        FConnectionStart = nullptr;
    }
}

void __fastcall TMainForm::miDeleteElementClick(TObject *Sender) {
    if (FSelectedElement) {
        // ������� ����������, ��������� � ���� ���������
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

        // ������� �������
        auto elemIt = std::find(FElements.begin(), FElements.end(), FSelectedElement);
        if (elemIt != FElements.end()) {
            delete *elemIt;
            FElements.erase(elemIt);
        }

        FSelectedElement = nullptr;
        UpdatePaintBoxSize();
        CircuitImage->Repaint();
        StatusBar->Panels->Items[0]->Text = "������� ������.";
    }
}

void __fastcall TMainForm::miPropertiesClick(TObject *Sender) {
    if (FSelectedElement) {
        ShowElementProperties(FSelectedElement);
    }
}

void TMainForm::ShowElementProperties(TCircuitElement* Element) {
    String info = "�������� ��������:\n";
    info += "���: " + Element->Name + "\n";
    info += "ID: " + IntToStr(static_cast<int>(Element->Id)) + "\n";
    info += "�������: (" + IntToStr(static_cast<int>(Element->Bounds.Left)) + ", " +
            IntToStr(static_cast<int>(Element->Bounds.Top)) + ")\n";
    info += "������: " + IntToStr(static_cast<int>(Element->Bounds.Width())) + " x " +
            IntToStr(static_cast<int>(Element->Bounds.Height())) + "\n";
    info += "�����: " + IntToStr(static_cast<int>(Element->Inputs.size())) + "\n";
    info += "������: " + IntToStr(static_cast<int>(Element->Outputs.size())) + "\n";

    // ���������� � ���������
    String stateStr;
    switch (Element->CurrentState) {
        case TTernary::NEG: stateStr = "-1"; break;
        case TTernary::ZERO: stateStr = "0"; break;
        case TTernary::POS: stateStr = "+1"; break;
    }
    info += "������� ���������: " + stateStr;

    Application->MessageBox(info.w_str(), L"�������� ��������", MB_OK | MB_ICONINFORMATION);
}

void __fastcall TMainForm::miRotateElementClick(TObject *Sender) {
    if (FSelectedElement) {
        // ������� ���������� �������� - ������ ������� ������ � ������
        TRect newBounds = FSelectedElement->Bounds;
        int width = newBounds.Width();
        int height = newBounds.Height();

        // ������ ������� (��� ���������� ����������)
        newBounds.Right = newBounds.Left + height;
        newBounds.Bottom = newBounds.Top + width;

        FSelectedElement->SetBounds(newBounds);
        UpdatePaintBoxSize();
        CircuitImage->Repaint();
        StatusBar->Panels->Items[0]->Text = "������� ��������.";
    }
}

void __fastcall TMainForm::btnZoomInClick(TObject *Sender) {
    FZoomFactor *= 1.2;
    ApplyZoom();
    StatusBar->Panels->Items[0]->Text = "����������: " + FloatToStrF(FZoomFactor * 100, ffFixed, 3, 0) + "%";
}

void __fastcall TMainForm::btnZoomOutClick(TObject *Sender) {
    FZoomFactor /= 1.2;
    if (FZoomFactor < 0.1) FZoomFactor = 0.1;
    ApplyZoom();
    StatusBar->Panels->Items[0]->Text = "����������: " + FloatToStrF(FZoomFactor * 100, ffFixed, 3, 0) + "%";
}

void __fastcall TMainForm::btnZoomFitClick(TObject *Sender) {
    FZoomFactor = 1.0;
    ApplyZoom();
    StatusBar->Panels->Items[0]->Text = "���������� �������� �� 100%";
}

void TMainForm::ApplyZoom() {
    // ������������ ��� ��������
    for (auto element : FElements) {
        TRect bounds = element->Bounds;
        bounds.Left = (int)(bounds.Left * FZoomFactor);
        bounds.Top = (int)(bounds.Top * FZoomFactor);
        bounds.Right = (int)(bounds.Right * FZoomFactor);
        bounds.Bottom = (int)(bounds.Bottom * FZoomFactor);
        element->SetBounds(bounds);
    }

    // ��������� ������ ������� ���������
    UpdatePaintBoxSize();
    CircuitImage->Repaint();
}
