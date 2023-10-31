#pragma once

#include <string>
#include <vector>
#include <map>
#include "data.h"


class Type {

public:

    Type() = default;
    explicit Type(std::string typeName, std::any defaultValue);
    void setConstructor(const Value& function);

    std::string name;
    std::any defaultValue;
    Value constructor;
    std::map<std::string, Value> members;

};