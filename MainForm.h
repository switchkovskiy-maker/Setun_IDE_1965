#ifndef MainFormH
#define MainFormH

#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ToolWin.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Menus.hpp>
#include "CircuitElement.h"
#include "CircuitElements.h"
#include <System.Classes.hpp>

class TMainForm : public TForm {
__published:
    TPanel *ToolPanel;
    TPanel *LibraryPanel;
    TPanel *WorkspacePanel;
    TScrollBox *Workspace;
    TPaintBox *CircuitImage;
    TStatusBar *StatusBar;
    TToolBar *MainToolBar;
    TButton *btnRunSimulation;
    TButton *btnResetSimulation;
    TButton *btnClearWorkspace;
    TSpeedButton *btnConnectionMode;
    TListBox *ElementLibrary;
    TLabel *LibraryLabel;
    TPopupMenu *ElementPopupMenu;
    TMenuItem *miDeleteElement;
    TMenuItem *miProperties;
    TMenuItem *N1;
    TMenuItem *miRotateElement;
    TButton *btnZoomIn;
    TButton *btnZoomOut;
    TButton *btnZoomFit;

    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall CircuitImageMouseDown(TObject *Sender, TMouseButton Button,
        TShiftState Shift, int X, int Y);
    void __fastcall CircuitImageMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall CircuitImageMouseUp(TObject *Sender, TMouseButton Button,
        TShiftState Shift, int X, int Y);
    void __fastcall CircuitImagePaint(TObject *Sender);
    void __fastcall btnRunSimulationClick(TObject *Sender);
    void __fastcall btnResetSimulationClick(TObject *Sender);
    void __fastcall btnClearWorkspaceClick(TObject *Sender);
    void __fastcall btnConnectionModeClick(TObject *Sender);
    void __fastcall ElementLibraryDblClick(TObject *Sender);
    void __fastcall miDeleteElementClick(TObject *Sender);
    void __fastcall miPropertiesClick(TObject *Sender);
    void __fastcall miRotateElementClick(TObject *Sender);
    void __fastcall btnZoomInClick(TObject *Sender);
    void __fastcall btnZoomOutClick(TObject *Sender);
    void __fastcall btnZoomFitClick(TObject *Sender);
    void __fastcall WorkspaceResize(TObject *Sender);

private:
    std::vector<TCircuitElement*> FElements;
    std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>> FConnections;
    TCircuitElement* FSelectedElement;
    TCircuitElement* FDraggedElement;
    TConnectionPoint* FConnectionStart;
    bool FIsConnecting;
    bool FIsDragging;
    int FNextElementId;
    int FDragOffsetX;
    int FDragOffsetY;
    double FZoomFactor;

    void DrawCircuit();
    TColor TernaryToColor(TTernary Value);
    void RunSimulationStep();
    void ResetSimulation();
    void CreateCompleteLibrary();
    void UpdateElementPositions();
    TCircuitElement* CreateElementByType(const String& ElementDesc, int X, int Y);
    void ShowElementProperties(TCircuitElement* Element);
    void UpdatePaintBoxSize();
    void ApplyZoom();
    TRect GetCircuitBounds();

public:
    __fastcall TMainForm(TComponent* Owner);
};

extern PACKAGE TMainForm *MainForm;
#endif
