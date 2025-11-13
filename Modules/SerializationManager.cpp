#include "MainForm.h"
#include "SerializationManager.h"

#pragma package(smart_init)

TSerializationManager::TSerializationManager(TMainForm* MainForm) 
    : FMainForm(MainForm) {
}

void TSerializationManager::SaveSchemeToFile(const String& FileName, TTabData* TabData) {
    if (!TabData) return;

    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    // Сохраняем основную информацию
    iniFile->WriteInteger("Scheme", "ElementCount", static_cast<int>(TabData->Elements.size()));
    iniFile->WriteInteger("Scheme", "ConnectionCount", static_cast<int>(TabData->Connections.size()));
    iniFile->WriteInteger("Scheme", "NextElementId", TabData->NextElementId);
    iniFile->WriteString("Scheme", "Version", "3.0");

    // Сохраняем элементы
    for (int i = 0; i < TabData->Elements.size(); i++) {
        String section = "Element_" + IntToStr(i);
        SaveElementToIni(TabData->Elements[i].get(), iniFile.get(), section);
    }

    // Сохраняем соединения
    for (int i = 0; i < TabData->Connections.size(); i++) {
        String section = "Connection_" + IntToStr(i);
        auto& connection = TabData->Connections[i];

        if (connection.first && connection.second) {
            SaveConnectionPoint(connection.first, iniFile.get(), section, "From");
            SaveConnectionPoint(connection.second, iniFile.get(), section, "To");
        }
    }

    iniFile->UpdateFile();
}

void TSerializationManager::LoadSchemeFromFile(const String& FileName, TTabData* TabData) {
    if (!TabData) return;

    std::unique_ptr<TIniFile> iniFile(new TIniFile(FileName));

    String version = iniFile->ReadString("Scheme", "Version", "1.0");
    if (version == "1.0") {
        throw Exception("Формат файла устарел. Используйте новую версию для сохранения схем.");
    }

    int elementCount = iniFile->ReadInteger("Scheme", "ElementCount", 0);
    int connectionCount = iniFile->ReadInteger("Scheme", "ConnectionCount", 0);
    TabData->NextElementId = iniFile->ReadInteger("Scheme", "NextElementId", 1);

    // Загружаем элементы
    std::map<int, TCircuitElement*> idToElementMap;

    for (int i = 0; i < elementCount; i++) {
        String section = "Element_" + IntToStr(i);
        auto element = LoadElementFromIni(iniFile.get(), section);

        if (element) {
            idToElementMap[element->Id] = element.get();
            TabData->Elements.push_back(std::move(element));
        }
    }

    // Загружаем соединения
    for (int i = 0; i < connectionCount; i++) {
        String section = "Connection_" + IntToStr(i);

        TConnectionPoint* fromPoint = LoadConnectionPoint(iniFile.get(), section, "From", nullptr);
        TConnectionPoint* toPoint = LoadConnectionPoint(iniFile.get(), section, "To", nullptr);

        if (fromPoint && toPoint) {
            // Находим реальные точки соединения в загруженных элементах
            TCircuitElement* fromElement = idToElementMap[fromPoint->Owner->Id];
            TCircuitElement* toElement = idToElementMap[toPoint->Owner->Id];

            TConnectionPoint* realFromPoint = FindConnectionPointInElement(fromElement, fromPoint);
            TConnectionPoint* realToPoint = FindConnectionPointInElement(toElement, toPoint);

            if (realFromPoint && realToPoint) {
                TabData->Connections.push_back(std::make_pair(realFromPoint, realToPoint));
            }

            delete fromPoint;
            delete toPoint;
        }
    }
}

void TSerializationManager::SaveElementToIni(TCircuitElement* Element, TIniFile* IniFile, const String& Section) {
    if (Element) {
        Element->SaveToIni(IniFile, Section);
    }
}

std::unique_ptr<TCircuitElement> TSerializationManager::LoadElementFromIni(TIniFile* IniFile, const String& Section) {
    String className = IniFile->ReadString(Section, "ClassName", "");
    int id = IniFile->ReadInteger(Section, "Id", 0);
    int x = IniFile->ReadInteger(Section, "X", 0);
    int y = IniFile->ReadInteger(Section, "Y", 0);

    auto element = CreateElementByClassName(className, id, x, y);
    if (element) {
        element->LoadFromIni(IniFile, Section);
    }

    return element;
}

void TSerializationManager::SaveConnectionPoint(const TConnectionPoint* Point, TIniFile* IniFile,
                                               const String& Section, const String& Prefix) {
    if (!Point || !Point->Owner) return;

    IniFile->WriteInteger(Section, Prefix + "ElementId", Point->Owner->Id);
    IniFile->WriteFloat(Section, Prefix + "RelX", Point->RelX);
    IniFile->WriteFloat(Section, Prefix + "RelY", Point->RelY);
    IniFile->WriteBool(Section, Prefix + "IsInput", Point->IsInput);
}

TConnectionPoint* TSerializationManager::LoadConnectionPoint(TIniFile* IniFile, const String& Section,
                                                            const String& Prefix, TCircuitElement* Owner) {
    int elementId = IniFile->ReadInteger(Section, Prefix + "ElementId", -1);
    if (elementId == -1) return nullptr;

    // Создаем временный элемент-заглушку для точки соединения
    auto tempElement = std::make_unique<TTempCircuitElement>(elementId);

    auto point = new TConnectionPoint(
        tempElement.get(),
        0, 0,
        TTernary::ZERO,
        IniFile->ReadBool(Section, Prefix + "IsInput", true),
        TLineStyle::POSITIVE_CONTROL
    );

    point->RelX = IniFile->ReadFloat(Section, Prefix + "RelX", 0);
    point->RelY = IniFile->ReadFloat(Section, Prefix + "RelY", 0);

    // Сохраняем временный элемент, чтобы избежать его уничтожения
    static std::vector<std::unique_ptr<TCircuitElement>> tempElements;
    tempElements.push_back(std::move(tempElement));

    return point;
}

TConnectionPoint* TSerializationManager::FindConnectionPointInElement(TCircuitElement* element, 
                                                                     const TConnectionPoint* pointTemplate) {
    if (!element) return nullptr;

    auto& points = pointTemplate->IsInput ? element->Inputs : element->Outputs;

    for (auto& point : points) {
        if (fabs(point.RelX - pointTemplate->RelX) < 0.001 &&
            fabs(point.RelY - pointTemplate->RelY) < 0.001) {
            return &point;
        }
    }

    return nullptr;
}

std::unique_ptr<TCircuitElement> TSerializationManager::CreateElementByClassName(const String& ClassName, 
                                                                                int Id, int X, int Y) {
    if (ClassName == "TMagneticAmplifier") {
        return std::make_unique<TMagneticAmplifier>(Id, X, Y, false);
    } else if (ClassName == "TTernaryElement") {
        return std::make_unique<TTernaryElement>(Id, X, Y);
    } else if (ClassName == "TShiftRegister") {
        return std::make_unique<TShiftRegister>(Id, X, Y, 4);
    } else if (ClassName == "TTernaryTrigger") {
        return std::make_unique<TTernaryTrigger>(Id, X, Y);
    } else if (ClassName == "THalfAdder") {
        return std::make_unique<THalfAdder>(Id, X, Y);
    } else if (ClassName == "TTernaryAdder") {
        return std::make_unique<TTernaryAdder>(Id, X, Y);
    } else if (ClassName == "TDecoder") {
        return std::make_unique<TDecoder>(Id, X, Y, 2);
    } else if (ClassName == "TCounter") {
        return std::make_unique<TCounter>(Id, X, Y, 3);
    } else if (ClassName == "TDistributor") {
        return std::make_unique<TDistributor>(Id, X, Y, 8);
    } else if (ClassName == "TSwitch") {
        return std::make_unique<TSwitch>(Id, X, Y, 3);
    } else if (ClassName == "TLogicAnd") {
        return std::make_unique<TLogicAnd>(Id, X, Y);
    } else if (ClassName == "TLogicOr") {
        return std::make_unique<TLogicOr>(Id, X, Y);
    } else if (ClassName == "TLogicInhibit") {
        return std::make_unique<TLogicInhibit>(Id, X, Y);
    } else if (ClassName == "TGenerator") {
        return std::make_unique<TGenerator>(Id, X, Y);
    } else if (ClassName == "TSubCircuit") {
        return std::make_unique<TSubCircuit>(Id, X, Y,
            std::vector<std::unique_ptr<TCircuitElement>>(),
            std::vector<std::pair<TConnectionPoint*, TConnectionPoint*>>());
    }

    return nullptr;
}