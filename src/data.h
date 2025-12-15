#pragma once

#include <string>
#include "macros.h"
#include <variant>
#include "types/type.h"
#include "types/function.h"

struct Value {

    Value() : type(nullptr), value(nullptr), thrownException(nullptr) {};

    Value(reference<Type>& type);

    Value(boolean a);
    Value(integer i);
    Value(real r);
    Value(string s);
    Value(Function function);

    boolean initialized = true;
    reference<Type> type;
    variant<real, string, integer, boolean, Function, void*> value;
    reference<Value> thrownException; //non-null when exception is thrown

};

const Value NullValue = Value();