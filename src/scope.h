#pragma once

#include <string>
#include <map>
#include "data.h"
#include "types/type.h"

class Scope {
public:
    explicit Scope(reference<Scope> parent = nullptr);

    // variables
    void setVariable(const std::string& name, const Value& value);
    bool hasVariable(const std::string& name) const;
    Value getVariable(const std::string& name);

    // types
    void addType(const std::string& name, reference<Type> type);
    reference<Type> getType(const std::string& name);

    // scope management
    reference<Scope> createChild();

    void printVariables() const;

private:
    reference<Scope> parent;

    std::map<std::string, Value> variables;
    std::map<std::string, reference<Type>> types;

    Value makeUndefinedVariableError(const std::string& name) const;
};
