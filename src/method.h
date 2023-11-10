#pragma once

#include "statement.h"
#include "data.h"
#include "type.h"

class Scope;

class Method {

public:

    class Argument {
    public:
        std::string name;
        Type type;
    };

    Value call(std::vector<Value> passedArgs, Scope& scope);

    int argCount;
    bool internal;
    Value (*internalMethod)(std::vector<Value>&);
    std::vector<Argument> arguments;
    Block block;

};