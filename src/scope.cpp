#include "scope.h"
#include <iostream>
#include "expressions.h"
#include "types/type.h"
#include "runtime_builtins.h"
#include "runtime_exception.h"

reference<Scope> makeScopeReference(Scope& scope) {
    return reference<Scope>(&scope, [](Scope*) {});
}

// =======================
// Constructor
// =======================
Scope::Scope(reference<Scope> parent, bool initializeBuiltins)
    : parent(parent) {

    // Only root scope defines builtins
    if (!parent && initializeBuiltins) {
        Type::defineTypes();
        addVariable("int", Value(IntegerType));
        addVariable("boolean", Value(BooleanType));
        addVariable("string", Value(StringType));
        addVariable("real", Value(RealType));
        addVariable("type", Value(TypeType));
        addVariable("function", Value(FunctionType));
        addVariable("any", Value(AnyType));
        registerBuiltinRuntime(*this);
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
    if (variables.contains(name) && variables[name] != nullptr) {
        *variables[name] = initial;
        return;
    }
    variables[name] = create_reference<Value>(initial);
}

void Scope::setVariable(const std::string& name, const Value& value) {
    if (DEBUG) {
        std::cout << "Setting variable " << name << " of type "
                  << (value.type ? value.type->getName() : "<null>")
                  << std::endl;
    }
    if (variables.contains(name)) {
        auto& cell = variables[name];
        if (cell == nullptr) cell = create_reference<Value>(value);
        else *cell = value;
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
        const auto& cell = variables.at(name);
        if (cell != nullptr) return *cell;
        return makeUndefinedVariableError(name);
    }

    // parent
    if (parent)
        return parent->getVariable(name);

    // undefined
    return makeUndefinedVariableError(name);
}

Scope Scope::createChild() {
    return Scope(makeScopeReference(*this));
}

void Scope::collectVisibleVariables(std::map<std::string, reference<Value>>& out) const {
    if (parent) {
        parent->collectVisibleVariables(out);
    }
    for (const auto& [name, value] : variables) {
        out[name] = value;
    }
}

Scope Scope::snapshot() const {
    Scope copy(nullptr, false);
    collectVisibleVariables(copy.variables);
    return copy;
}

// =======================
// Errors
// =======================
Value Scope::makeUndefinedVariableError(const std::string& name) const {
    return makeThrown("NameException", "undefined variable: " + name);
}

// =======================
// Debug
// =======================
void Scope::printVariables() const {
    std::cout << "Scope variables:\n";
    for (const auto& [name, value] : variables) {
        if (value == nullptr) {
            std::cout << "  " << name << " = <null>\n";
            continue;
        }
        std::cout << "  " << name << " = " << stringify(*value) << "\n";
    }
}
