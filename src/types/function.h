#pragma once

#include "type.h"
#include <vector>

class Block;
class Value;
class Scope;

struct FunctionParameter {
    string name;
    reference<Type> type;
};

struct Function {
    reference<Value> __this;
    std::vector<FunctionParameter> parameters;
    reference<Scope> closure;
    reference<Block> body;

    bool validArguments(const std::vector<Value>& arguments);
};