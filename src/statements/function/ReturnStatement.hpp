#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"
#include <vector>

class ReturnStatement : public Statement {
public:

    static uref<ReturnStatement> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:
    uref<Expression> expression;
};
