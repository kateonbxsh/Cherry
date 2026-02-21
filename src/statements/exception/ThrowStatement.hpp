#pragma once

#include "statement.h"
#include "statements/expression/Expression.h"

class ThrowStatement : public Statement {
public:
    static uref<ThrowStatement> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    Token throwToken;
    uref<Expression> expression;
};

