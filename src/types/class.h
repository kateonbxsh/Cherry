#pragma once

#include "type.h"
#include <unordered_map>
#include "function.h"

struct Method {
    std::vector<Function> overloads;
};

enum FieldFlags {
    Public = 1,
    Private = 2,
    Protected = 4,
    Static = 8,
    Readonly = 16
};

struct Field {
    std::string name;
    reference<Expression> type;
    unsigned int flags;
    bool hasDefaultValue;
    reference<Expression> value;
};

struct TypeParameter {
    string name;
    reference<Type> value;
    bool hasDefault;
    reference<Type> defaultValue;
};

class ClassInstance {

private:

    std::unordered_map<string, Value> fieldValues;

};