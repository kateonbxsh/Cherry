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
        addVariable("int", Value(IntegerType));
        addVariable("boolean", Value(BooleanType));
        addVariable("string", Value(StringType));
        addVariable("real", Value(RealType));
        addVariable("type", Value(TypeType));
        addVariable("function", Value(FunctionType));
    }
}

// =======================
// Variables
// =======================
void Scope::addVariable(const std::string& name, const Value& initial) {
    if (DEBUG) {
        std::cout << "Adding variable " << name << " of type "
                  << (initial.type ? initial.type->getName() : "<null>")
                  << std::endl;
    }
    variables[name] = initial;
}

void Scope::setVariable(const std::string& name, const Value& value) {
    if (DEBUG) {
        std::cout << "Setting variable " << name << " of type "
                  << (value.type ? value.type->getName() : "<null>")
                  << std::endl;
    }
    if (variables.contains(name)) {
        variables[name] = value;
        return;
    }
    if (parent) {
        parent->setVariable(name, value);
    }
    return;
    
}

bool Scope::hasVariable(const std::string& name) const {
    if (variables.contains(name))
        return true;

    if (parent)
        return parent->hasVariable(name);

    return false;
}

Value Scope::getVariable(const std::string& name) {
    
    if (DEBUG) std::cout << "Getting variable " << name << std::endl;;
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

Scope Scope::createChild() {
    reference<Scope> self(this, [](Scope*) {});
    return Scope(self);
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
