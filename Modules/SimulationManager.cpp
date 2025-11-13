#include "SimulationManager.h"
#include "MainForm.h"

#pragma package(smart_init)

TSimulationManager::TSimulationManager() 
    : FSimulationRunning(false), FSimulationStep(0), FCurrentTab(nullptr) {
    
    FSimulationTimer = new TTimer(nullptr);
    FSimulationTimer->Interval = 500;
    FSimulationTimer->OnTimer = &SimulationTimerTimer;
    FSimulationTimer->Enabled = false;
}

TSimulationManager::~TSimulationManager() {
    FSimulationTimer->Enabled = false;
    delete FSimulationTimer;
}

void TSimulationManager::SetCurrentTab(TTabData* Tab) {
    FCurrentTab = Tab;
}

void TSimulationManager::RunSimulationStep() {
    if (!FCurrentTab) return;

    // Передача значений через соединения
    for (auto& connection : FCurrentTab->Connections) {
        if (connection.first && connection.second) {
            connection.second->Value = connection.first->Value;
        }
    }

    // Вычисление состояний элементов
    for (auto& element : FCurrentTab->Elements) {
        element->Calculate();
    }

    FSimulationStep++;
}

void TSimulationManager::ResetSimulation() {
    if (!FCurrentTab) return;

    for (auto& element : FCurrentTab->Elements) {
        // Сброс специальных элементов
        TTernaryTrigger* trigger = dynamic_cast<TTernaryTrigger*>(element.get());
        if (trigger) {
            trigger->Reset();
        }

        TCounter* counter = dynamic_cast<TCounter*>(element.get());
        if (counter) {
            counter->Reset();
        }

        // Сброс состояний входов/выходов
        for (auto& input : element->Inputs) {
            input.Value = TTernary::ZERO;
        }
        for (auto& output : element->Outputs) {
            output.Value = TTernary::ZERO;
        }
    }

    // Сброс соединений
    for (auto& connection : FCurrentTab->Connections) {
        if (connection.first) connection.first->Value = TTernary::ZERO;
        if (connection.second) connection.second->Value = TTernary::ZERO;
    }

    FSimulationStep = 0;
}

void TSimulationManager::StartSimulation() {
    FSimulationRunning = true;
    FSimulationTimer->Enabled = true;
}

void TSimulationManager::StopSimulation() {
    FSimulationRunning = false;
    FSimulationTimer->Enabled = false;
}

void __fastcall TSimulationManager::SimulationTimerTimer(TObject* Sender) {
    RunSimulationStep();
}