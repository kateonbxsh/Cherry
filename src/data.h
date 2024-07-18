#pragma once

#include <string>
#include <any>

struct Value {

    Type* type;
    void* value;
    bool thrown;

};