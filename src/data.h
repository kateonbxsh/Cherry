#pragma once

#include <string>
#include <any>
#include "type.h"

struct Value {

    const Type* type;
    void* value;
    bool thrown;

};

const Value NullValue = Value{nullptr, nullptr, false};