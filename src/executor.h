#pragma once

#include "scope.h"
#include "lexer.h"
#include "statement.h"

class Executor {

public:
    Executor() = default;

    static Value execute(Block& block);
    static Value evaluateExpression(TokenList rpnExpression, Scope& scope);

    static Value executeInternal(Block block, Scope& scope);

};