#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"

class VariableAffectation : public Statement {

public:

    static uref<VariableAffectation> parse(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    uref<Expression> expression;
    Token name;
    TokenKind selfOperation;

};
