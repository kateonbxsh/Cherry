#pragma once

#include <string>
#include <map>
#include <set>
#include "data.h"
#include "type.h"
#include "method.h"

class Scope {

public:
    Scope();

    void setVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name);
    bool hasVariable(const std::string& name);
    void setMethod(const std::string& name, Method method);
    bool hasMethod(const std::string& name);
    Method getMethod(const std::string& name);
    void addType(const Type& type);
    std::vector<Type> getTypes();

    void printVariables();

private:
    std::map<std::string, Value> variables;
    std::map<std::string, Method> methods;
    std::vector<Type> types;

};