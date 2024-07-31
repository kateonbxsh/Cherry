#pragma once

#include <string>
#include "macros.h"
#include <any>
#include "types/type.h"

struct Value {

    Value() : type(nullptr), value(nullptr), thrownException(nullptr) {}

    template<typename T>
    Value(reference<Type> type, T value)
        : type(type), value(value), thrownException(nullptr) {}

    reference<Type> type;
    variant<real, string, integer, boolean, void*> value;
    reference<Value> thrownException; //non-null when exception is thrown

};

const Value NullValue = Value(nullptr, nullptr);