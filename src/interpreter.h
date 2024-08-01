#pragma once

#include "statements/GlobalBlock.h"

class Interpreter {

public:
    Interpreter() = default;

    void interpret(uref<GlobalBlock>& block);

};