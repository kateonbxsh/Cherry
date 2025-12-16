#include "scope.h"
#include <iostream>
#include "expressions.h"

// =======================
// Constructor
// =======================
Scope::Scope(reference<Scope> parent)
    : parent(parent) {

    // Only root scope defines builtins
    if (!parent) {
        addType("int", IntegerType);
        addType("boolean", BooleanType);
        addType("string", StringType);
        addType("real", RealType);
    }
}

// =======================
// Variables
// =======================
void Scope::setVariable(const std::string& name, const Value& value) {
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
        if (!val.initialized)
            return NullValue;
        return val;
    }

    // parent
    if (parent)
        return parent->getVariable(name);

    // undefined
    return makeUndefinedVariableError(name);
}

// =======================
// Types
// =======================
void Scope::addType(const std::string& name, reference<Type> type) {
    types[name] = type;
}

reference<Type> Scope::getType(const std::string& name) {
    if (types.contains(name))
        return types[name];

    if (parent)
        return parent->getType(name);

    return nullptr;
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
