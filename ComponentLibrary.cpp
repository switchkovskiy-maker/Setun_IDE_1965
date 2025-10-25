#include "ComponentLibrary.h"
#include <algorithm>

TComponentLibrary::TComponentLibrary(const String& Name, const String& Description)
    : FName(Name), FDescription(Description) {
}

void TComponentLibrary::RegisterElement(const String& Description, 
                                      std::function<TCircuitElement*(int, int, int)> Factory) {
    FElementDescriptions.push_back(Description);
    FElementFactories.push_back(Factory);
}

TCircuitElement* TComponentLibrary::CreateElement(const String& Description, int Id, int X, int Y) const {
    for (size_t i = 0; i < FElementDescriptions.size(); i++) {
        if (FElementDescriptions[i] == Description) {
            try {
                return FElementFactories[i](Id, X, Y);
            } 
            catch (Exception &e) {
                throw Exception("Ошибка создания элемента '" + Description + "': " + e.Message);
            }
            catch (...) {
                throw Exception("Неизвестная ошибка при создании элемента '" + Description + "'");
            }
        }
    }
    throw Exception("Элемент '" + Description + "' не найден в библиотеке");
}

bool TComponentLibrary::HasElement(const String& Description) const {
    return std::find(FElementDescriptions.begin(), FElementDescriptions.end(), Description) 
           != FElementDescriptions.end();
}

TLibraryManager::TLibraryManager() : FCurrentLibrary(nullptr) {
}

TLibraryManager::~TLibraryManager() {
    FLibraries.clear();
}

void TLibraryManager::RegisterLibrary(std::unique_ptr<TComponentLibrary> Library) {
    if (!Library) {
        throw Exception("Попытка зарегистрировать пустую библиотеку");
    }
    
    if (!FCurrentLibrary) {
        FCurrentLibrary = Library.get();
    }
    FLibraries.push_back(std::move(Library));
}

void TLibraryManager::UnregisterLibrary(const String& Name) {
    auto it = std::find_if(FLibraries.begin(), FLibraries.end(),
        [&Name](const std::unique_ptr<TComponentLibrary>& lib) {
            return lib->Name == Name;
        });
    
    if (it != FLibraries.end()) {
        if (FCurrentLibrary == it->get()) {
            FCurrentLibrary = !FLibraries.empty() ? FLibraries[0].get() : nullptr;
        }
        FLibraries.erase(it);
    }
}

void TLibraryManager::SetCurrentLibrary(const String& Name) {
    auto it = std::find_if(FLibraries.begin(), FLibraries.end(),
        [&Name](const std::unique_ptr<TComponentLibrary>& lib) {
            return lib->Name == Name;
        });
    
    if (it != FLibraries.end()) {
        FCurrentLibrary = it->get();
    } else {
        throw Exception("Библиотека '" + Name + "' не найдена");
    }
}

TComponentLibrary* TLibraryManager::GetCurrentLibrary() const {
    return FCurrentLibrary;
}

TComponentLibrary* TLibraryManager::GetLibrary(const String& Name) const {
    auto it = std::find_if(FLibraries.begin(), FLibraries.end(),
        [&Name](const std::unique_ptr<TComponentLibrary>& lib) {
            return lib->Name == Name;
        });
    
    return it != FLibraries.end() ? it->get() : nullptr;
}

std::vector<String> TLibraryManager::GetLibraryNames() const {
    std::vector<String> names;
    for (const auto& lib : FLibraries) {
        names.push_back(lib->Name);
    }
    return names;
}

TCircuitElement* TLibraryManager::CreateElementFromCurrent(const String& Description, 
                                                         int Id, int X, int Y) const {
    if (!FCurrentLibrary) {
        throw Exception("Текущая библиотека не установлена");
    }
    return FCurrentLibrary->CreateElement(Description, Id, X, Y);
}