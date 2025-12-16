#pragma once

#include "type.h"
#include <vector>

class Block;
class Value;

struct FunctionParameter {
    string name;
    reference<Type> type; 
};

struct Function {
    reference<Value> __this;
    std::vector<FunctionParameter> parameters;
    reference<Block> body; 
};