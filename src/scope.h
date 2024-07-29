#pragma once

#include <string>
#include <map>
#include <set>
#include "data.h"
#include "types/type.h"

class Scope {

public:
    Scope();

    void setVariable(const std::string& name, const Value& value);
    bool hasVariable(const std::string& name);
    Value getVariable(const std::string& name);

    void addType(const std::string& name, Type* type);
    Type* getType(const std::string& name);

    void printVariables();

private:
    std::map<std::string, Value> variables = {};
    std::map<std::string, Type*> types = {};

};