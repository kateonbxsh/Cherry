#pragma once

#include <string>
#include <any>

struct Value {

    std::string type;
    std::any value;
    bool thrown;

};