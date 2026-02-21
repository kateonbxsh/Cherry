#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"
#include <vector>

class FunctionCall : public Expression {
public:

    static uref<FunctionCall> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

    uref<Expression> function;
    std::vector<uref<Expression>> arguments;
    Token callToken;
    
};
