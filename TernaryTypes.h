#ifndef TernaryTypesH
#define TernaryTypesH

#include <System.Types.hpp>
#include <vector>
#include <map>

class TCircuitElement;

enum class TTernary { NEG = -1, ZERO = 0, POS = 1 };

enum class TElementType {
    MAGNETIC_AMPLIFIER,          
    MAGNETIC_AMPLIFIER_POWER,    
    TERNARY_ELEMENT,             
    TERNARY_TRIGGER,             
    TERNARY_ADDER,               
    HALF_ADDER,                  
    SHIFT_REGISTER,              
    DECODER,                     
    COUNTER,                     
    DISTRIBUTOR,                 
    SWITCH,                      
    LOGIC_AND,                   
    LOGIC_OR,                    
    LOGIC_INHIBIT,               
    GENERATOR,                   
    SUBCIRCUIT                   
};

enum class TLineStyle {
    POSITIVE_CONTROL,    
    NEGATIVE_CONTROL,    
    OUTPUT_LINE,         
    INTERNAL_CONNECTION  
};

struct TConnectionPoint {
    TCircuitElement* Owner;
    int X, Y;
    TTernary Value;
    bool IsInput;
    TLineStyle LineStyle;
    double RelX, RelY;
    
    TConnectionPoint(TCircuitElement* owner, int x, int y, TTernary val, bool isInput, TLineStyle style) 
        : Owner(owner), X(x), Y(y), Value(val), IsInput(isInput), LineStyle(style), RelX(0), RelY(0) {}
        
    TConnectionPoint(int x, int y, TTernary val, bool isInput, TLineStyle style) 
        : Owner(nullptr), X(x), Y(y), Value(val), IsInput(isInput), LineStyle(style), RelX(0), RelY(0) {}
};

#endif