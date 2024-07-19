#pragma once

#include <string>
#include <any>
#include "type.h"

struct Value {

    Type* type;
    void* value;
    bool thrown;

};