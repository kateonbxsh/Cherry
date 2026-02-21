#pragma once

#include <string>
#include <map>
#include "data.h"

class Scope {
public:
    explicit Scope(reference<Scope> parent = nullptr);

    // variables
    void addVariable(const std::string& name, const Value& initial);
    void setVariable(const std::string& name, const Value& value);
    bool hasVariable(const std::string& name) const;
    Value getVariable(const std::string& name);
    Scope createChild();

    void printVariables() const;

private:
    reference<Scope> parent;

    std::map<std::string, Value> variables;

    Value makeUndefinedVariableError(const std::string& name) const;
};
