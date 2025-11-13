#ifndef SimulationManagerH
#define SimulationManagerH

#include "CircuitElement.h"
#include "CircuitElements.h"
#include <System.Classes.hpp>
#include <Vcl.ExtCtrls.hpp>

class TTabData;

class TSimulationManager {
private:
    bool FSimulationRunning;
    int FSimulationStep;
    TTimer* FSimulationTimer;
    TTabData* FCurrentTab;

public:
    TSimulationManager();
    ~TSimulationManager();

    void SetCurrentTab(TTabData* Tab);
    void RunSimulationStep();
    void ResetSimulation();
    void StartSimulation();
    void StopSimulation();
    bool IsRunning() const { return FSimulationRunning; }
    int GetSimulationStep() const { return FSimulationStep; }

    void __fastcall SimulationTimerTimer(TObject* Sender);
};

#endif