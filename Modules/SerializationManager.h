#ifndef SerializationManagerH
#define SerializationManagerH

#include "CircuitElement.h"
#include "CircuitElements.h"
#include "ComponentLibrary.h"
#include <System.IniFiles.hpp>
#include <memory>
#include <map>

// Временный класс для загрузки соединений
class TTempCircuitElement : public TCircuitElement {
public:
    TTempCircuitElement(int AId) : TCircuitElement(AId, "Temp", 0, 0) {}
    void Calculate() override {
        // Пустая реализация для временного элемента
    }
};

class TTabData;
class TMainForm;

class TSerializationManager {
private:
    TMainForm* FMainForm;

public:
    TSerializationManager(TMainForm* MainForm);

    void SaveSchemeToFile(const String& FileName, TTabData* TabData);
    void LoadSchemeFromFile(const String& FileName, TTabData* TabData);

    void SaveElementToIni(TCircuitElement* Element, TIniFile* IniFile, const String& Section);
    std::unique_ptr<TCircuitElement> LoadElementFromIni(TIniFile* IniFile, const String& Section);

    void SaveConnectionPoint(const TConnectionPoint* Point, TIniFile* IniFile,
                           const String& Section, const String& Prefix);
    TConnectionPoint* LoadConnectionPoint(TIniFile* IniFile, const String& Section,
                                        const String& Prefix, TCircuitElement* Owner);

    TConnectionPoint* FindConnectionPointInElement(TCircuitElement* element,
                                                 const TConnectionPoint* pointTemplate);
    std::unique_ptr<TCircuitElement> CreateElementByClassName(const String& ClassName,
                                                            int Id, int X, int Y);
};

#endif
