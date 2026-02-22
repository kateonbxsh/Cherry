#pragma once

#include <string>
#include <map>
#include "data.h"

class Scope {
public:
    explicit Scope(reference<Scope> parent = nullptr, bool initializeBuiltins = true);

    // variables
    void addVariable(const std::string& name, const Value& initial);
    void setVariable(const std::string& name, const Value& value);
    bool hasVariable(const std::string& name) const;
    Value getVariable(const std::string& name);
    Scope createChild();
    Scope snapshot() const;

    void printVariables() const;

private:
    void collectVisibleVariables(std::map<std::string, reference<Value>>& out) const;
    reference<Scope> parent;

    std::map<std::string, reference<Value>> variables;

    Value makeUndefinedVariableError(const std::string& name) const;
};

reference<Scope> makeScopeReference(Scope& scope);
