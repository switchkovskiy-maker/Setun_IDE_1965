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
#include <System.IniFiles.hpp>
#include "CircuitElement.h"
#include "CircuitElements.h"
#include "ComponentLibrary.h"
#include <System.Classes.hpp>
#include <System.JSON.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Buttons.hpp>
#include <windows.h>

// Тип функции для регистрации библиотеки в DLL
typedef bool (__stdcall *TRegisterLibraryFunction)(TLibraryManager*);
// Тип функции для отмены регистрации библиотеки в DLL
typedef void (__stdcall *TUnregisterLibraryFunction)(TLibraryManager*);

class TMainForm : public TForm {
__published:
    TPanel *ToolPanel;
    TPanel *LibraryPanel;
    TPanel *WorkspacePanel;
    TScrollBox *Workspace;
    TPaintBox *CircuitImage;
    TStatusBar *StatusBar;
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
    TMenuItem *miGroupElements;
    TMenuItem *miUngroupElements;
    TSaveDialog *SaveDialog;
    TOpenDialog *OpenDialog;
    TComboBox *cmbLibrarySelector;
    TLabel *lblLibrarySelector;
    TMainMenu *MainMenu;
    TMenuItem *miFile;
    TMenuItem *miSave;
    TMenuItem *miLoad;
    TMenuItem *N2;
    TMenuItem *miExit;
    TMenuItem *miEdit;
    TMenuItem *miGroup;
    TMenuItem *miUngroup;
    TMenuItem *miView;
    TMenuItem *miZoomIn;
    TMenuItem *miZoomOut;
    TMenuItem *miZoomFit;
    TMenuItem *miSimulation;
    TMenuItem *miRun;
    TMenuItem *miReset;
    TMenuItem *miHelp;
    TMenuItem *miAbout;

    void __fastcall WorkspaceMouseWheel(TObject *Sender, TShiftState Shift,
    int WheelDelta, TPoint &MousePos, bool &Handled);
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
    void __fastcall btnSaveSchemeClick(TObject *Sender);
    void __fastcall btnLoadSchemeClick(TObject *Sender);
    void __fastcall btnGroupElementsClick(TObject *Sender);
    void __fastcall btnUngroupElementsClick(TObject *Sender);
    void __fastcall cmbLibrarySelectorChange(TObject *Sender);
    void __fastcall miExitClick(TObject *Sender);
    void __fastcall miAboutClick(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);

private:
    std::vector<std::unique_ptr<TCircuitElement>> FElements;
    std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>> FConnections;
    TCircuitElement* FSelectedElement;
    std::vector<TCircuitElement*> FSelectedElements;
    TCircuitElement* FDraggedElement;
    TConnectionPoint* FConnectionStart;
    bool FIsConnecting;
    bool FIsDragging;
    bool FIsSelecting;
    TRect FSelectionRect;
    int FNextElementId;
    int FDragOffsetX;
    int FDragOffsetY;
    double FZoomFactor;
    int FScrollOffsetX;
    int FScrollOffsetY;
    HINSTANCE FStandardLibraryHandle;
    std::unique_ptr<TLibraryManager> FLibraryManager;
    std::unique_ptr<TComponentLibrary> FBasicLibrary;

    void DrawCircuit();
    TColor TernaryToColor(TTernary Value);
    void RunSimulationStep();
    void ResetSimulation();
    void CreateCompleteLibrary();
    std::unique_ptr<TCircuitElement> CreateElement(const String& LibraryName, const String& ElementName, int X, int Y);
    std::unique_ptr<TCircuitElement> CreateElementFromCurrent(const String& ElementName, int X, int Y);
    void ShowElementProperties(TCircuitElement* Element);
    void UpdatePaintBoxSize();
    void ApplyZoom();
    void CenterCircuit();
    TRect GetCircuitBounds();
    void SaveSchemeToFile(const String& FileName);
    void LoadSchemeFromFile(const String& FileName);
    std::vector<TCircuitElement*> GetSelectedElements();
    void CreateSubCircuitFromSelection();
    void UngroupSubCircuit(TCircuitElement* SubCircuit);
    void UpdateLibrarySelector();
    void LoadCurrentLibrary();
    bool LoadStandardLibrary();
    void UnloadStandardLibrary();
    void CreateBasicLibrary();
    TPoint ScreenToLogical(const TPoint& screenPoint) const;
    TPoint LogicalToScreen(const TPoint& logicalPoint) const;
    TRect LogicalToScreen(const TRect& logicalRect) const;
    void DeleteSelectedElements();

    // Новые методы для размещения элементов
    TPoint GetVisibleAreaCenter() const;
    bool FindFreeLocation(int& x, int& y, int width, int height);
    TPoint GetBestPlacementPosition(int width, int height);

public:
    __fastcall TMainForm(TComponent* Owner);
};

class TSubCircuit : public TCircuitElement {
private:
    std::vector<std::unique_ptr<TCircuitElement>> FInternalElements;
    std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>> FInternalConnections;

public:
    TSubCircuit(int AId, int X, int Y,
                std::vector<std::unique_ptr<TCircuitElement>>&& Elements,
                const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& Connections);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;

    const std::vector<std::unique_ptr<TCircuitElement>>& GetInternalElements() const { return FInternalElements; }
    const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& GetInternalConnections() const { return FInternalConnections; }

private:
    void CreateExternalConnections();
    void UpdateExternalConnections();
};

extern PACKAGE TMainForm *MainForm;

#endif
