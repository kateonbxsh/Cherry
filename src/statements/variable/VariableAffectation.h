#pragma once

#include "statement.h"
#include "lexer.h"
#include "statements/expression/Expression.h"
#include <vector>

class VariableAffectation : public Statement {

public:

    static uref<VariableAffectation> parse(Lexer& lexer);
    static uref<VariableAffectation> parseWithoutSemicolon(Lexer& lexer);
    Value execute(Scope& scope) override;

private:

    uref<Expression> leftTarget;
    uref<Expression> expression;
    TokenKind selfOperation;

};
