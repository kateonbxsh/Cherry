#pragma once

#include <string>
#include <map>
#include <set>
#include "data.h"
#include "types/type.h"

class Scope {

public:
    Scope();

    void setVariable(const string& name, const Value& value);
    bool hasVariable(const string& name);
    Value getVariable(const string& name);

    void addType(const string& name, reference<Type> type);
    reference<Type> getType(const string& name);

    void printVariables();

private:
    std::map<string, Value> variables = {};
    std::map<string, reference<Type>> types = {};

};