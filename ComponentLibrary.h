#ifndef ComponentLibraryH
#define ComponentLibraryH

#include "CircuitElement.h"
#include "CircuitElements.h"
#include <vector>
#include <memory>
#include <functional>
#include <System.Classes.hpp>

class TComponentLibrary {
private:
    String FName;
    String FDescription;
    std::vector<String> FElementDescriptions;
    std::vector<std::function<TCircuitElement*(int, int, int)>> FElementFactories;

public:
    TComponentLibrary(const String& Name, const String& Description);
    
    void RegisterElement(const String& Description, 
                        std::function<TCircuitElement*(int, int, int)> Factory);
    
    TCircuitElement* CreateElement(const String& Description, int Id, int X, int Y) const;
    bool HasElement(const String& Description) const;
    
    __property String Name = { read = FName };
    __property String Description = { read = FDescription };
    __property std::vector<String> ElementDescriptions = { read = FElementDescriptions };
};

class TLibraryManager {
private:
    std::vector<std::unique_ptr<TComponentLibrary>> FLibraries;
    TComponentLibrary* FCurrentLibrary;

public:
    TLibraryManager();
    ~TLibraryManager();
    
    void RegisterLibrary(std::unique_ptr<TComponentLibrary> Library);
    void UnregisterLibrary(const String& Name);
    void SetCurrentLibrary(const String& Name);
    
    TComponentLibrary* GetCurrentLibrary() const;
    TComponentLibrary* GetLibrary(const String& Name) const;
    std::vector<String> GetLibraryNames() const;
    
    TCircuitElement* CreateElementFromCurrent(const String& Description, int Id, int X, int Y) const;
};

#endif