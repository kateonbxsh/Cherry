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

    static Value Uninitialized(reference<Type>& type);

    reference<Type> type;
    variant<real, string, integer, boolean, Function, reference<Type>, ClassInstance, void*> value;
    reference<Value> thrownException; //non-null when exception is thrown
    boolean returning = false; // returning from current method

};

const Value NullValue = Value();