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
#include "Modules/SimulationManager.h"
#include "Modules/SerializationManager.h"
#include <System.Classes.hpp>
#include <System.JSON.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Buttons.hpp>
#include <windows.h>
#include <vector>
#include <map>

// Тип функции для регистрации библиотеки в DLL
typedef bool (__stdcall *TRegisterLibraryFunction)(TLibraryManager*);
// Тип функции для отмены регистрации библиотеки в DLL
typedef void (__stdcall *TUnregisterLibraryFunction)(TLibraryManager*);

// Структура для хранения информации о загруженной библиотеке
struct TLoadedLibrary {
    HINSTANCE Handle;
    String FileName;
    String LibraryName;
    TUnregisterLibraryFunction UnregisterFunc;
};

// Структура для хранения данных вкладки
struct TTabData {
    TScrollBox* ScrollBox;
    TPaintBox* PaintBox;
    std::vector<std::unique_ptr<TCircuitElement>> Elements;
    std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>> Connections;
    bool IsSubCircuit;
    bool IsReadOnly;
    TCircuitElement* SubCircuit;
    int NextElementId;

    TTabData() : ScrollBox(nullptr), PaintBox(nullptr), IsSubCircuit(false),
                 IsReadOnly(false), SubCircuit(nullptr), NextElementId(1) {}
    ~TTabData() {
        // Автоматическая очистка при удалении
    }
};
// Предварительное объявление
class TSubCircuit;

class TMainForm : public TForm {
__published:
    TPanel *ToolPanel;
    TPanel *LibraryPanel;
    TPanel *WorkspacePanel;
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
    TMenuItem *miOptions;
    TMenuItem *miRectangularConnections;
    TMenuItem *miSnapToGrid;
    TMenuItem *miSimulation;
    TMenuItem *miRun;
    TMenuItem *miReset;
    TMenuItem *miHelp;
    TMenuItem *miAbout;
    TPageControl *SchemePageControl;
    TPopupMenu *TabPopupMenu;
    TMenuItem *miCloseTab;
    TMenuItem *miViewSubCircuit;
    TMenuItem *miShowBridges;
    TMenuItem *miExport;
    TMenuItem *miExportVerilog;
    TMenuItem *miExportQuartus;

    void __fastcall miExportVerilogClick(TObject *Sender);
    void __fastcall miExportQuartusClick(TObject *Sender);
    void __fastcall CircuitImageMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall CircuitImageMouseUp(TObject *Sender, TMouseButton Button,
        TShiftState Shift, int X, int Y);
    void __fastcall WorkspaceMouseWheel(TObject *Sender, TShiftState Shift,
        int WheelDelta, const TPoint &MousePos, bool &Handled);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall CircuitImageMouseDown(TObject *Sender, TMouseButton Button,
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
    void __fastcall SchemePageControlChange(TObject *Sender);
    void __fastcall miCloseTabClick(TObject *Sender);
    void __fastcall miViewSubCircuitClick(TObject *Sender);
    void __fastcall miRectangularConnectionsClick(TObject *Sender);
    void __fastcall miSnapToGridClick(TObject *Sender);
    void __fastcall SchemePageControlDrawTab(TCustomTabControl *Control, int TabIndex, const TRect &Rect, bool Active);
    void __fastcall SchemePageControlMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall SchemePageControlMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall miShowBridgesClick(TObject *Sender);
private:
    // Структура для хранения информации о сегментах соединений
    struct TConnectionSegment {
        TPoint Start;
        TPoint End;
        TColor Color;
        bool IsHorizontal;
    };

    friend class TSubCircuit;
    // Методы для доступа к менеджерам (только для дружественных классов)
    TSerializationManager* GetSerializationManager() { return FSerializationManager.get(); }
	TSimulationManager* GetSimulationManager() { return FSimulationManager.get(); }

	// Менеджеры
    std::unique_ptr<TSimulationManager> FSimulationManager;
    std::unique_ptr<TSerializationManager> FSerializationManager;

    // Элементы интерфейса и состояния
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

    // Библиотеки
    std::unique_ptr<TLibraryManager> FLibraryManager;
    std::unique_ptr<TComponentLibrary> FBasicLibrary;
    std::vector<TLoadedLibrary> FLoadedLibraries;

    // Настройки интерфейса
    int FHotCloseTabIndex;
    bool FRectangularConnections;
    bool FSnapToGrid;
    bool FShowBridges;
    bool FIsDrawingWire;
    std::vector<TPoint> FCurrentWirePoints;
    TConnectionPoint* FWireStartPoint;

    // Основные методы отрисовки и управления
    void DrawCircuit();
    void CreateCompleteLibrary();
    std::unique_ptr<TCircuitElement> CreateElement(const String& LibraryName, const String& ElementName, int X, int Y);
    std::unique_ptr<TCircuitElement> CreateElementFromCurrent(const String& ElementName, int X, int Y);
    void ShowElementProperties(TCircuitElement* Element);
    void UpdatePaintBoxSize();
    void ApplyZoom();
    void CenterCircuit();
    TRect GetCircuitBounds();
    std::vector<TCircuitElement*> GetSelectedElements();
    void CreateSubCircuitFromSelection();
    void UngroupSubCircuit(TCircuitElement* SubCircuit);
    void UpdateLibrarySelector();
    void LoadCurrentLibrary();
    void CreateBasicLibrary();
    TPoint ScreenToLogical(const TPoint& screenPoint) const;
    TPoint LogicalToScreen(const TPoint& logicalPoint) const;
    TRect LogicalToScreen(const TRect& logicalRect) const;
    void DeleteSelectedElements();

    // Методы позиционирования
    TPoint GetVisibleAreaCenter() const;
    bool FindFreeLocation(int& x, int& y, int width, int height);
    TPoint GetBestPlacementPosition(int width, int height);

    // Методы восстановления состояний
    TConnectionPoint* FindRestoredConnectionPoint(const TConnectionPoint* originalPoint);
    void OptimizedDrawCircuit(TCanvas* Canvas, TTabData* TabData);

    // Методы работы с библиотеками
    void LoadAllLibraries();
    void UnloadAllLibraries();
    bool LoadLibraryFromDLL(const String& DllPath);
    TRegisterLibraryFunction FindRegisterFunction(HINSTANCE LibraryHandle);
    TUnregisterLibraryFunction FindUnregisterFunction(HINSTANCE LibraryHandle);

    // Методы управления вкладками
    TTabSheet* CreateNewTab(const String& Title, TSubCircuit* SubCircuit = nullptr);
    void UpdateCurrentTab();
    TTabData* GetCurrentTabData() const;
    void CloseTab(TTabSheet* Tab);
    void UpdateTabTitle(TTabSheet* Tab, const String& Title);
    void UpdateTabWidths();
    TConnectionPoint* FindConnectionPointByRelCoords(TCircuitElement* element, double relX, double relY, bool isInput);
    TRect GetTabCloseButtonRect(TCustomTabControl *Control, int TabIndex, const TRect &TabRect);
    bool IsPointInTabCloseButton(TCustomTabControl *Control, int TabIndex, int X, int Y);
    
    // Методы рисования соединений
    void DrawRectangularConnection(TCanvas* Canvas, const TPoint& Start, const TPoint& End, TColor Color);
    TPoint SnapToGridPoint(const TPoint& Point);

    // Методы рисования проводов
    void StartWireDrawing(TConnectionPoint* StartPoint);
    void AddWirePoint(const TPoint& Point);
    void CompleteWireDrawing(TConnectionPoint* EndPoint = nullptr);

    // Методы для работы с путями соединений
    std::vector<TPoint> CalculateSmartPath(const TPoint& Start, const TPoint& End) const;
    std::vector<TPoint> CalculateRectangularPath(const TPoint& Start, const TPoint& End) const;
    TColor TernaryToColor(TTernary Value) const;
    bool DoSegmentsIntersect(const TPoint& A1, const TPoint& A2, const TPoint& B1, const TPoint& B2) const;
    TPoint FindIntersectionPoint(const TPoint& A1, const TPoint& A2, const TPoint& B1, const TPoint& B2) const;
    std::vector<TConnectionSegment> GetAllConnectionSegments(TTabData* TabData) const;
    void AddBridgeToPath(std::vector<TPoint>& Path, const TPoint& Intersection, bool IsHorizontal);

    // Методы экспорта
    void ExportToVerilog(const String& FileName);
    void ExportToQuartusProject(const String& ProjectDir);
    String GenerateVerilogModule(TCircuitElement* Element, int& ModuleCount);
    String GenerateVerilogWires(TTabData* TabData);
    String GeneratePinAssignments(TTabData* TabData);
    String TritToVerilogCode(TTernary Value);

protected:
    friend class TSubCircuit;

public:
    __fastcall TMainForm(TComponent* Owner);
};

// Объявление TSubCircuit остается без изменений
class TSubCircuit : public TCircuitElement {
private:
    std::vector<std::unique_ptr<TCircuitElement>> FInternalElements;
    std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>> FInternalConnections;
    TTabSheet* FAssociatedTab;

public:
    TSubCircuit(int AId, int X, int Y,
                std::vector<std::unique_ptr<TCircuitElement>>&& Elements,
                const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& Connections);
    void Calculate() override;
    void Draw(TCanvas* Canvas) override;

    const std::vector<std::unique_ptr<TCircuitElement>>& GetInternalElements() const { return FInternalElements; }
    const std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>& GetInternalConnections() const { return FInternalConnections; }

    void SetAssociatedTab(TTabSheet* Tab) { FAssociatedTab = Tab; }
    TTabSheet* GetAssociatedTab() const { return FAssociatedTab; }

    virtual String GetClassName() const override { return "TSubCircuit"; }
    virtual void SaveToIni(TIniFile* IniFile, const String& Section) const override;
    virtual void LoadFromIni(TIniFile* IniFile, const String& Section) override;

private:
    void CreateExternalConnections();
    void UpdateExternalConnections();
    TCircuitElement* FindInternalElementById(int Id);
    TConnectionPoint* FindConnectionPointInInternalElements(const TConnectionPoint* pointTemplate);
};

extern PACKAGE TMainForm *MainForm;

#endif