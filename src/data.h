#pragma once

#include <string>
#include <any>

typedef struct {

    std::string type;
    std::any value;
    bool thrown;

} Value;