#pragma once

#include "macros.h"
#include <functional>
#include <vector>

class Block;
class Value;
class Scope;
class Type;

struct FunctionParameter {
    string name;
    reference<Type> type;
    bool variadic = false;
};

enum class FunctionKind {
    User,
    Internal
};

struct Function {
    reference<Value> __this;
    std::vector<FunctionParameter> parameters;
    reference<Scope> closure;
    reference<Block> body;
    FunctionKind kind = FunctionKind::User;
    std::function<Value(Scope&, const std::vector<Value>&, const reference<Value>&)> internalHandler;

    bool validArguments(const std::vector<Value>& arguments) const;
};
