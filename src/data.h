#pragma once

#include <string>
#include <any>
#include <type.h>

struct Value {

    Value(Type* type, void* value): type(type), value(value), thrownException(nullptr) {};

    Type* type;
    void* value;
    Value* thrownException; //non-null when exception is thrown

};

const Value NullValue = Value(nullptr, nullptr);