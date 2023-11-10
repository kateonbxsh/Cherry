#pragma once

#include "../statement.h"
#include "../type.h"
#include "../lexer.h"

class FunctionCall : public Statement {

public:
    FunctionCall(): Statement(FUNCTION_CALL) {};

    TokenList expression;
};
