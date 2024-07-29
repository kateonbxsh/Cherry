#pragma once

#include <string>
#include <any>
#include "types/type.h"

struct Value {

    Value(): type(nullptr), value(nullptr), thrownException(nullptr) {};
    Value(const Type *type, void* value): type(type), value(value), thrownException(nullptr) {};

    const Type* type;
    void* value;
    Value* thrownException; //non-null when exception is thrown

};

const Value NullValue = Value(nullptr, nullptr);