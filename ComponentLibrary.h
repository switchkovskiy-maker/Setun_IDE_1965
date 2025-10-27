#ifndef ComponentLibraryH
#define ComponentLibraryH

#include "CircuitElement.h"
#include "CircuitElements.h"
#include <vector>
#include <memory>
#include <functional>
#include <System.Classes.hpp>

// ������� ����� ��� ���� ��������� ����������
class TLibraryElement {
public:
    virtual ~TLibraryElement() = default;
    virtual String GetName() const = 0;
    virtual String GetDescription() const = 0;
    virtual String GetCategory() const = 0;
    virtual std::unique_ptr<TCircuitElement> CreateElement(int Id, int X, int Y) const = 0;
    virtual void GetDefaultSize(int& Width, int& Height) const = 0;
};

// ���������� ���������� �������� ����������
template<typename T>
class TConcreteLibraryElement : public TLibraryElement {
private:
    String FName;
    String FDescription;
    String FCategory;
    int FDefaultWidth;
    int FDefaultHeight;

public:
    TConcreteLibraryElement(const String& Name, const String& Description,
                          const String& Category, int DefaultWidth = 80, int DefaultHeight = 60)
        : FName(Name), FDescription(Description), FCategory(Category),
          FDefaultWidth(DefaultWidth), FDefaultHeight(DefaultHeight) {}

    String GetName() const override { return FName; }
    String GetDescription() const override { return FDescription; }
    String GetCategory() const override { return FCategory; }

    std::unique_ptr<TCircuitElement> CreateElement(int Id, int X, int Y) const override {
        return std::make_unique<T>(Id, X, Y);
    }

    void GetDefaultSize(int& Width, int& Height) const override {
        Width = FDefaultWidth;
        Height = FDefaultHeight;
    }
};

// ���������� �����������
class TComponentLibrary {
private:
    String FName;
    String FDescription;
    String FVersion;
    std::vector<std::unique_ptr<TLibraryElement>> FElements;
    std::map<String, TLibraryElement*> FElementMap;

    // ����� ��� ��������� ���������� ���������
    int GetElementCount() const { return static_cast<int>(FElements.size()); }

public:
    TComponentLibrary(const String& Name, const String& Description, const String& Version = "1.0");

    // ����������� ���������
    template<typename T>
    void RegisterElement(const String& Name, const String& Description,
                        const String& Category = "General", int DefaultWidth = 80, int DefaultHeight = 60) {
        auto element = std::make_unique<TConcreteLibraryElement<T>>(Name, Description, Category, DefaultWidth, DefaultHeight);
        FElementMap[Name] = element.get();
        FElements.push_back(std::move(element));
    }

    // �������� ��������
    std::unique_ptr<TCircuitElement> CreateElement(const String& Name, int Id, int X, int Y) const;

    // ����� ��������
    bool HasElement(const String& Name) const;
    const TLibraryElement* GetElementInfo(const String& Name) const;

    // ��������� �������
    std::vector<String> GetElementNames() const;
    std::vector<String> GetElementNamesByCategory(const String& Category) const;
    std::vector<String> GetCategories() const;

    __property String Name = { read = FName };
    __property String Description = { read = FDescription };
    __property String Version = { read = FVersion };
    __property int ElementCount = { read = GetElementCount };
};

// �������� ���������
class TLibraryManager {
private:
    std::vector<std::unique_ptr<TComponentLibrary>> FLibraries;
    TComponentLibrary* FCurrentLibrary;
    std::map<String, TComponentLibrary*> FLibraryMap;

public:
    TLibraryManager();
    ~TLibraryManager();

    void RegisterLibrary(std::unique_ptr<TComponentLibrary> Library);
    void UnregisterLibrary(const String& Name);
    void SetCurrentLibrary(const String& Name);

    TComponentLibrary* GetCurrentLibrary() const;
    TComponentLibrary* GetLibrary(const String& Name) const;
    std::vector<String> GetLibraryNames() const;

    // �������� ���������
    std::unique_ptr<TCircuitElement> CreateElement(const String& LibraryName, const String& ElementName, int Id, int X, int Y) const;
    std::unique_ptr<TCircuitElement> CreateElementFromCurrent(const String& ElementName, int Id, int X, int Y) const;

    // ����� ���������
    bool HasElement(const String& LibraryName, const String& ElementName) const;
    const TLibraryElement* GetElementInfo(const String& LibraryName, const String& ElementName) const;
};

#endif
