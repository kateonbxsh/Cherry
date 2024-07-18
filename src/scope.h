#pragma once

#include <string>
#include <map>
#include <set>
#include "data.h"
#include "type.h"

class Scope {

public:
    Scope();

    void setVariable(const std::string& name, const Value& value);
    bool hasVariable(const std::string& name);
    Value getVariable(const std::string& name);

    void addType(const std::string& name, const Type& type);
    std::vector<Type> getTypes();

    void printVariables();

private:
    std::map<std::string, Value> variables;
    std::map<std::string, Type> types;

};