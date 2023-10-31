#pragma once

#include "statement.h"
#include "data.h"
#include "scope.h"
#include "lexer.h"

class Executor {

public:
    Executor() = default;

    static Value execute(Block& block);
    static Value evaluateExpression(TokenList rpnExpression, Scope& scope);

private:

    static Value executeInternal(Block& block, Scope& scope);

};