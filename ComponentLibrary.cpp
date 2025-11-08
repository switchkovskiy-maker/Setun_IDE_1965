#include "ComponentLibrary.h"
#include <algorithm>
#include <map>

#pragma package(smart_init)

// Реализация TComponentLibrary
TComponentLibrary::TComponentLibrary(const String& Name, const String& Description, const String& Version)
    : FName(Name), FDescription(Description), FVersion(Version) {
}

std::unique_ptr<TCircuitElement> TComponentLibrary::CreateElement(const String& Name, int Id, int X, int Y) const {
    auto it = FElementMap.find(Name);
    if (it != FElementMap.end()) {
        return it->second->CreateElement(Id, X, Y);
    }
    throw Exception("Элемент '" + Name + "' не найден в библиотеке '" + FName + "'");
}

bool TComponentLibrary::HasElement(const String& Name) const {
    return FElementMap.find(Name) != FElementMap.end();
}

const TLibraryElement* TComponentLibrary::GetElementInfo(const String& Name) const {
    auto it = FElementMap.find(Name);
    return it != FElementMap.end() ? it->second : nullptr;
}

std::vector<String> TComponentLibrary::GetElementNames() const {
    std::vector<String> names;
    for (const auto& element : FElements) {
        names.push_back(element->GetName());
    }
    return names;
}

std::vector<String> TComponentLibrary::GetElementNamesByCategory(const String& Category) const {
    std::vector<String> names;
    for (const auto& element : FElements) {
        if (element->GetCategory() == Category) {
            names.push_back(element->GetName());
        }
    }
    return names;
}

std::vector<String> TComponentLibrary::GetCategories() const {
    std::map<String, bool> categories;
    for (const auto& element : FElements) {
        categories[element->GetCategory()] = true;
    }

    std::vector<String> result;
    for (const auto& pair : categories) {
        result.push_back(pair.first);
    }
    return result;
}

// Реализация TLibraryManager
TLibraryManager::TLibraryManager() : FCurrentLibrary(nullptr) {
}

TLibraryManager::~TLibraryManager() {
    FLibraries.clear();
    FLibraryMap.clear();
}

void TLibraryManager::RegisterLibrary(std::unique_ptr<TComponentLibrary> Library) {
    if (!Library) {
        throw Exception("Попытка зарегистрировать пустую библиотеку");
    }

    String libraryName = Library->Name;
    if (FLibraryMap.find(libraryName) != FLibraryMap.end()) {
        throw Exception("Библиотека '" + libraryName + "' уже зарегистрирована");
    }

    if (!FCurrentLibrary) {
        FCurrentLibrary = Library.get();
    }

    FLibraryMap[libraryName] = Library.get();
    FLibraries.push_back(std::move(Library));
}

void TLibraryManager::UnregisterLibrary(const String& Name) {
    auto mapIt = FLibraryMap.find(Name);
    if (mapIt != FLibraryMap.end()) {
        FLibraryMap.erase(mapIt);
    }

    auto libIt = std::find_if(FLibraries.begin(), FLibraries.end(),
        [&Name](const std::unique_ptr<TComponentLibrary>& lib) {
            return lib->Name == Name;
        });

    if (libIt != FLibraries.end()) {
        if (FCurrentLibrary == libIt->get()) {
            FCurrentLibrary = !FLibraries.empty() ? FLibraries[0].get() : nullptr;
        }
        FLibraries.erase(libIt);
    }
}

void TLibraryManager::SetCurrentLibrary(const String& Name) {
    auto it = FLibraryMap.find(Name);
    if (it != FLibraryMap.end()) {
        FCurrentLibrary = it->second;
    } else {
        throw Exception("Библиотека '" + Name + "' не найдена");
    }
}

TComponentLibrary* TLibraryManager::GetCurrentLibrary() const {
    return FCurrentLibrary;
}

TComponentLibrary* TLibraryManager::GetLibrary(const String& Name) const {
    auto it = FLibraryMap.find(Name);
    return it != FLibraryMap.end() ? it->second : nullptr;
}

std::vector<String> TLibraryManager::GetLibraryNames() const {
    std::vector<String> names;
    for (const auto& pair : FLibraryMap) {
        names.push_back(pair.first);
    }
    return names;
}

std::unique_ptr<TCircuitElement> TLibraryManager::CreateElement(const String& LibraryName, const String& ElementName, int Id, int X, int Y) const {
    auto library = GetLibrary(LibraryName);
    if (!library) {
        throw Exception("Библиотека '" + LibraryName + "' не найдена");
    }
    return library->CreateElement(ElementName, Id, X, Y);
}

std::unique_ptr<TCircuitElement> TLibraryManager::CreateElementFromCurrent(const String& ElementName, int Id, int X, int Y) const {
    if (!FCurrentLibrary) {
        throw Exception("Текущая библиотека не установлена");
    }
    return FCurrentLibrary->CreateElement(ElementName, Id, X, Y);
}

bool TLibraryManager::HasElement(const String& LibraryName, const String& ElementName) const {
    auto library = GetLibrary(LibraryName);
    return library && library->HasElement(ElementName);
}

const TLibraryElement* TLibraryManager::GetElementInfo(const String& LibraryName, const String& ElementName) const {
    auto library = GetLibrary(LibraryName);
    return library ? library->GetElementInfo(ElementName) : nullptr;
}
