#include "scope.h"
#include <iostream>
#include "expressions.h"
#include "types/type.h"

// =======================
// Constructor
// =======================
Scope::Scope(reference<Scope> parent)
    : parent(parent) {

    // Only root scope defines builtins
    if (!parent) {
        Type::defineTypes();
        setVariable("int", Value(IntegerType));
        setVariable("boolean", Value(BooleanType));
        setVariable("string", Value(StringType));
        setVariable("real", Value(RealType));
        setVariable("type", Value(TypeType));
    }
}

// =======================
// Variables
// =======================
void Scope::setVariable(const std::string& name, const Value& value) {
    if (DEBUG) std::cout << "Setting variable " << name << " of type " << value.type->getName() << std::endl;;
    variables[name] = value;
}

bool Scope::hasVariable(const std::string& name) const {
    if (variables.contains(name))
        return true;

    if (parent)
        return parent->hasVariable(name);

    return false;
}

Value Scope::getVariable(const std::string& name) {
    // local
    if (variables.contains(name)) {
        const Value& val = variables.at(name);
        return val;
    }

    // parent
    if (parent)
        return parent->getVariable(name);

    // undefined
    return makeUndefinedVariableError(name);
}

// =======================
// Errors
// =======================
Value Scope::makeUndefinedVariableError(const std::string& name) const {
    Value err = NullValue;
    err.thrownException = create_reference<Value>(
        Value("undefined variable: " + name)
    );
    return err;
}

// =======================
// Debug
// =======================
void Scope::printVariables() const {
    std::cout << "Scope variables:\n";
    for (const auto& [name, value] : variables) {
        std::cout << "  " << name << " = " << stringify(value) << "\n";
    }
}
