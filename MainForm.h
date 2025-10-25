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
#include <System.Classes.hpp>
#include <System.JSON.hpp>
#include <Vcl.Dialogs.hpp>

// Предварительное объявление класса подсхемы
class TSubCircuit;

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
    TSpeedButton *btnMultiSelect;  // Добавляем кнопку множественного выделения
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
    TButton *btnSaveScheme;
    TButton *btnLoadScheme;
    TButton *btnGroupElements;
    TButton *btnUngroupElements;
    TSaveDialog *SaveDialog;
    TOpenDialog *OpenDialog;
    
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
    void __fastcall btnMultiSelectClick(TObject *Sender);  // Обработчик множественного выделения
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
    
private:
    std::vector<TCircuitElement*> FElements;
    std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>> FConnections;
    TCircuitElement* FSelectedElement;
    std::vector<TCircuitElement*> FSelectedElements;  // Множественное выделение
    TCircuitElement* FDraggedElement;
    TConnectionPoint* FConnectionStart;
    bool FIsConnecting;
    bool FIsDragging;
    bool FIsMultiSelecting;  // Режим множественного выделения
    TRect FSelectionRect;    // Прямоугольник выделения
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
    TCircuitElement* CreateElementByTypeName(const String& TypeName, int X, int Y);
    void ShowElementProperties(TCircuitElement* Element);
    void UpdatePaintBoxSize();
    void ApplyZoom();
    TRect GetCircuitBounds();
    void SaveSchemeToFile(const String& FileName);
    void LoadSchemeFromFile(const String& FileName);
    String ElementTypeToString(TElementType Type);
    TElementType StringToElementType(const String& TypeStr);
    std::vector<TCircuitElement*> GetSelectedElements();
    void CreateSubCircuitFromSelection();
    void UngroupSubCircuit(TCircuitElement* SubCircuit);
    
public:
    __fastcall TMainForm(TComponent* Owner);
};

// Класс подсхемы
class TSubCircuit : public TCircuitElement {
private:
    std::vector<TCircuitElement*> FInternalElements;
    std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>> FInternalConnections;
    
public:
    TSubCircuit(int AId, int X, int Y, const std::vector<TCircuitElement*>& Elements, 
                const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& Connections);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;
    
    // Геттеры для доступа к внутренним данным
    const std::vector<TCircuitElement*>& GetInternalElements() const { return FInternalElements; }
    const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& GetInternalConnections() const { return FInternalConnections; }
    
private:
    void CreateExternalConnections();
    void UpdateExternalConnections();
};

extern PACKAGE TMainForm *MainForm;
#endif